/* -*- C -*- 
   mboxgrep - scan mailbox for messages matching a regular expression
   Copyright (C) 2000, 2001, 2002, 2003  Daniel Spiljar

   Mboxgrep is free software; you can redistribute it and/or modify it 
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Mboxgrep is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mboxgrep; if not, write to the Free Software Foundation, 
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: mbox.c,v 1.26 2003/08/24 19:23:50 dspiljar Exp $ */

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <errno.h>
#include <time.h>
#ifdef HAVE_FLOCK
#include <sys/file.h>
#endif /* HAVE_FLOCK */
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
#include <bzlib.h>
#endif /* HAVE_LIBBZ2 */

#include "mboxgrep.h"
#include "mbox.h"
#include "misc.h"
#include "wrap.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

mbox_t *
mbox_open (const char *path, const char *mode)
     /* {{{  */
{
  mbox_t *mp;
  static int fd;
#ifndef HAVE_FLOCK
  struct flock lck;
#endif /* HAVE_FLOCK */
  char buffer[BUFSIZ];

  mp = (mbox_t *) xmalloc (sizeof (mbox_t));
  mp->postmark_cache = (char *) xmalloc (BUFSIZ * sizeof (char));

  if (0 == strcmp ("-", path))
    mp->fp = stdin;
  else
    {
      if (mode[0] == 'r')
	fd = m_open (path, O_RDONLY, 0);
      else if (mode[0] == 'w')
	fd = m_open (path, (O_WRONLY | O_CREAT | O_APPEND),
		     (S_IWUSR | S_IRUSR));
      else
	{
	  fprintf (stderr, "%s: mbox.c: Unknown mode %c.  You shouldn't "
		   "get this error...", APPNAME, mode[0]);
	  exit (2);
	}

      if (fd == -1)
	{
	  if (config.merr)
	    {
	      fprintf (stderr, "%s: %s: ", APPNAME, path);
	      perror (NULL);
	    }
	  errno = 0;
	  return NULL;
	}
      
      if (config.lock)
	{
#ifdef HAVE_FLOCK
	  int op;

	  if (mode[0] == 'r')
	    op = LOCK_SH;
	  else
	    op = LOCK_EX;
	  if (-1 == flock (fd, op))
#else
	    memset (&lck, 0, sizeof (struct flock));
	  lck.l_whence = SEEK_SET;
	  if (mode[0] == 'r')
	    lck.l_type = F_RDLCK;
	  else
	    lck.l_type = F_WRLCK;

	  if (-1 == fcntl (fd, F_SETLK, &lck))
#endif /* HAVE_FLOCK */
	    {
	      if (config.merr)
		{
		  fprintf (stderr, "%s: %s: ", APPNAME, path);
		  perror (NULL);
		}
	      errno = 0;
	      close (fd);
	      return NULL;
	    }
	}

      if (mode[0] == 'r')
	{
	  if (config.format == MBOX)
	    mp->fp = (FILE *) m_fdopen (fd, "r");
#ifdef HAVE_LIBZ
	  else if (config.format == ZMBOX)
	    mp->fp = (gzFile *) m_gzdopen (fd, "rb");
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
	  else if (config.format == BZ2MBOX)
	    mp->fp = (BZFILE *) BZ2_bzdopen (fd, "rb");
#endif /* HAVE_LIBBZ2 */
	}
      else if (mode[0] == 'w')
	{
	  if (config.format == MBOX)
	    mp->fp = (FILE *) m_fdopen (fd, "w");
#ifdef HAVE_LIBZ
	  else if (config.format == ZMBOX)
	    mp->fp = (gzFile *) m_gzdopen (fd, "wb");
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
	  else if (config.format == BZ2MBOX)
	    mp->fp = (BZFILE *) BZ2_bzdopen (fd, "wb");
#endif /* HAVE_LIBBZ2 */
	}
      
      if (mp->fp == NULL)
	{
	  if (config.merr)
	    {
	      fprintf (stderr, "%s: %s: ", APPNAME, path);
	      perror (NULL);
	    }
	  errno = 0;
	  close (fd);
	  return NULL;
	}
    }

  if (mode[0] == 'r')
    {
      if (config.format == MBOX)
	fgets (buffer, BUFSIZ, mp->fp);
#ifdef HAVE_LIBZ
      else if (config.format == ZMBOX)
	gzgets (mp->fp, buffer, BUFSIZ);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
      else if (config.format == BZ2MBOX)
	{
	  char c[1] = "\0";
	  int n = 0;

	  while (c[0] != '\n' && n < BUFSIZ)
	    {
	      BZ2_bzread (mp->fp, c, 1);
	      buffer[n] = c[0];
	      n++;
	    }
	  buffer[n] = '\0';
	}
#endif /* HAVE_LIBBZ2 */

      if (0 != strncmp ("From ", buffer, 5))
	{
	  if (config.merr)
	    {
	      if (0 == strcmp ("-", path))
		fprintf (stderr, "%s: (standard input): Not a mbox folder\n", 
			 APPNAME);
	      else
		fprintf (stderr, "%s: %s: Not a mbox folder\n", APPNAME, path);
	    }
	  if (config.format == MBOX)
	    fclose (mp->fp);
#ifdef HAVE_LIBZ
	  else if (config.format == ZMBOX)
	    gzclose (mp->fp);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
	  else if (config.format == BZ2MBOX)
	    BZ2_bzclose (mp->fp);
#endif /* HAVE_LIBBZ2 */
	  return NULL;
	}
      strcpy (mp->postmark_cache, buffer);
    }
  return mp;
}
/* }}} */

void
mbox_close (mbox_t * mp)
     /* {{{  */
{
  if (config.format == MBOX)
    fclose (mp->fp);
#ifdef HAVE_LIBZ
  else if (config.format == ZMBOX)
    gzclose (mp->fp);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  else if (config.format == BZ2MBOX)
    BZ2_bzclose (mp->fp);
#endif /* HAVE_LIBBZ2 */

  free (mp->postmark_cache);
  free (mp);
}
/* }}} */

message_t *
mbox_read_message (mbox_t * mp)
     /* {{{  */

{
  int isheaders = 1, s;
  char buffer[BUFSIZ];
  message_t *message;

  message = (message_t *) xmalloc (sizeof (message_t));

  message->headers = (char *) xmalloc (sizeof (char));
  message->headers[0] = '\0';
  message->hbytes = 0;

  message->body = (char *) xmalloc (sizeof (char));
  message->body[0] = '\0';
  message->bbytes = 0;

  message->from = NULL;

  s = strlen (mp->postmark_cache);
  message->headers =
    (char *) realloc (message->headers,
                      ((1 + s + message->hbytes) * sizeof (char)));
  strcpy (message->headers + message->hbytes, mp->postmark_cache);
  message->hbytes += s;

  for (;;)
    {
      if (config.format == MBOX)
	{
	  if (fgets (buffer, BUFSIZ, mp->fp) == NULL)
	    {
	      if (isheaders)
		return NULL;
	      else
		return message;
	    }
	}

#ifdef HAVE_LIBZ
      else if (config.format == ZMBOX)
	{
	  if (gzgets (mp->fp, buffer, BUFSIZ) == NULL)
	    {
	      if (isheaders)
		return NULL;
	      else
		return message;
	    }
	}
#endif /* HAVE_LIBZ */

#ifdef HAVE_LIBBZ2
      else if (config.format == BZ2MBOX)
	{
	  char c[1] = "\0";
	  int n = 0;

	  while (c[0] != '\n' && n < BUFSIZ)
	    {
	      BZ2_bzread (mp->fp, c, 1);
	      buffer[n] = c[0];
	      n++;
	    }
	  buffer[n] = '\0';

	  if (buffer[0] == '\0')
	    {
	      if (isheaders)
		return NULL;
	      else
		return message;
	    }
	}
#endif /* HAVE_LIBBZ2 */

      s = strlen (buffer);

      if (buffer[0] == '\n' && isheaders == 1)
	{
	  isheaders = 0;
	  continue;
	}

      if (isheaders)
	{
	  message->headers =
	    (char *) realloc (message->headers,
			      ((1 + s + message->hbytes) * sizeof (char)));
	  strcpy (message->headers + message->hbytes, buffer);
	  message->hbytes += s;
	}			/* if */
      else
	{
	  if (0 == strncmp (buffer, "From ", 5))
	    {
	      strcpy (mp->postmark_cache, buffer);
	      return message;
	    }
	  message->body =
	    (char *) realloc (message->body,
			      ((1 + s + message->bbytes) * sizeof (char)));
	  strcpy (message->body + message->bbytes, buffer);
	  message->bbytes += s;

	}
    }				/* for */
  return NULL;
}

/* }}} */

void *
tmpfile_open (const char *path)
     /* {{{  */

{
  extern char *tmpfilename;
  char *fname;
  char *tmpdir;
  int foo;

  if (path == NULL) { /* no path prefix given, use /tmp or TMPDIR */
    tmpdir = getenv ("TMPDIR");
    if (tmpdir == NULL)
      tmpdir = xstrdup ("/tmp");
    fname = xstrdup ("/mboxgrepXXXXXX");
  }
  else {
    tmpdir = (char *)path;
    fname = xstrdup (".XXXXXX");
  }

  tmpfilename = (char *) xmalloc ((strlen (tmpdir) + (strlen (fname) + 1))
				    * sizeof (char));
  sprintf (tmpfilename, "%s%s", tmpdir, fname);
  foo = mkstemp (tmpfilename);
  if (-1 == foo)
    {
      if (config.merr)
	{
	  fprintf (stderr, "%s: %s: ", APPNAME, tmpfilename);
	  perror (NULL);
	}
      exit (2);
    }

  if (config.format == MBOX)
    return (m_fdopen (foo, "w"));
#ifdef HAVE_LIBZ
  else if (config.format == ZMBOX)
    return (m_gzdopen (foo, "wb"));
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  else if (config.format == BZ2MBOX)
    return (BZ2_bzdopen (foo, "wb"));
#endif /* HAVE_LIBZ */

  return NULL; /* not reached */
}

/* }}} */

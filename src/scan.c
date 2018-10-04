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
   
   $Id: scan.c,v 1.21 2003/04/06 21:01:49 dspiljar Exp $ */

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif /* HAVE_SYS_NDIR_H */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* HAVE_SYS_DIR_H */
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif /* HAVE_NDIR_H */
#endif /* HAVE_DIRENT_H */

#include <time.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_LIBZ
# include <zlib.h>
#define BUFLEN 16384
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBPCRE
# include <pcre.h>
#endif /* HAVE_LIBPCRE */

#include "scan.h"
#include "mbox.h"
#include "mh.h"
#include "maildir.h"
#include "wrap.h"
#include "md5.h"
#ifdef HAVE_FTS_OPEN
# include <sys/stat.h>
# include <fts.h>
#else
# ifdef HAVE_FTW
#  include <ftw.h>
# endif /* HAVE_FTW */
#endif /* HAVE_FTS_OPEN */

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

void scan_mailbox (char path[])
     /* {{{  */
{
  static FILE *outf;
  extern FILE *tmpp;
  static mbox_t *mbox, *out;
#ifdef HAVE_LIBPCRE
  extern pcre *pcre_pattern;
  extern pcre_extra *hints;
  int of[BUFSIZ];
#endif /* HAVE_LIBPCRE */
  static DIR *boxd, *foo;
  static maildir_t *maildird;
  static message_t *msg;
  extern regex_t posix_pattern;
  extern char *pipecmd, *outboxname;
  extern int count;
  int delete = 0;
  char date_str[80];
  int isdup = 0;
  time_t tt;
  struct tm *ct;
  extern checksum_t *cs;

  extern option_t config;


  if (config.format == MAILDIR && config.action == WRITE)
    {
      foo = opendir (outboxname); /* do NOT change this to m_opendir! */
      if (foo == NULL && errno == ENOENT)
	maildir_create (outboxname);
      else closedir (foo);

      if (-1 == maildir_check (outboxname))
	{
	  if (config.merr)
	    fprintf (stderr, "%s: %s: Not a maildir folder\n", APPNAME, 
		     outboxname);
	  exit (2);
	}
    }

  count = 0;
  if (config.action == DELETE)
    delete = 1;

  if ((config.format == MBOX) || (config.format == ZMBOX) ||
      (config.format == BZ2MBOX))
    {
      mbox = (mbox_t *) mbox_open (path, "r");
      if (mbox == NULL) return;
    }
  else if ((config.format == MH) || (config.format == NNMH) ||
	   (config.format == NNML))
    {
      boxd = mh_open (path);
      if (boxd == NULL) return;
    }
  else if (config.format == MAILDIR)
    {
      maildird = maildir_open (path);
      if (maildird == NULL) return;
    }

  for (;;)
    {
      int res1 = 1, res2 = 1;

      if ((config.format == MBOX) || (config.format == ZMBOX) ||
	  (config.format == BZ2MBOX))
	msg = (message_t *) mbox_read_message (mbox);
      else if ((config.format == MH) || (config.format == NNMH) ||
	       (config.format == NNML))
	msg = (message_t *) mh_read_message (boxd);
      else if (config.format == MAILDIR)
	msg = (message_t *) maildir_read_message (maildird);

      if (msg == NULL) break;

      if (msg->from == NULL) msg->from = (char *) xstrdup ("nobody");

#ifdef HAVE_LIBPCRE
      if (config.perl)
	{
	  if (config.headers)
	    res1 = pcre_exec (pcre_pattern, hints, msg->headers,
			      (int) strlen (msg->headers), 0, 0, of, BUFSIZ);
	  if (config.body)
	    res2 = pcre_exec (pcre_pattern, hints, msg->body,
			      (int) strlen (msg->body), 0, 0, of, BUFSIZ);

	  res1 = res1 ^ 1;
	  res2 = res2 ^ 1;
	}
      else
#endif /* HAVE_LIBPCRE */
	{
	  if (config.headers)
	    res1 = regexec (&posix_pattern, msg->headers, 0, NULL, 0);
	  if (config.body)
	    res2 = regexec (&posix_pattern, msg->body, 0, NULL, 0);
	}

      if (config.dedup)
	isdup = md5_check_message (msg->body, cs);

      if (((res1 == 0) | (res2 == 0)) ^ ((config.invert ^ delete)) &&
	  ((config.dedup && !isdup) || !config.dedup))
	{
	  if (config.action == DISPLAY)
	    {
	      if (config.format != MBOX && config.format != ZMBOX
		  && config.format != BZ2MBOX
		  && 0 != strncmp ("From ", msg->headers, 5))
		{
		  tt = time (NULL);
		  ct = localtime (&tt);
		  strftime (date_str, 80, "%a %b %d %H:%M:%S %Y", ct);
		  if (msg->from)
		    fprintf (stdout, "From %s  %s\n", msg->from, date_str);
		  else
		    fprintf (stdout, "From nobody  %s\n", date_str);
		}
	      fprintf (stdout, "%s\n%s", msg->headers, msg->body);
	    }
	  else if (config.action == WRITE)
	    {
	      if (config.format == MAILDIR)
		maildir_write_message (msg, outboxname);
	      else if (config.format == MH || config.format == NNMH ||
		       config.format == NNML)
		mh_write_message (msg, outboxname);
	      else if (config.format == MBOX)
		{
		  out = mbox_open (outboxname, "w");
		  fprintf (out->fp, "%s\n%s", msg->headers, msg->body);
		  mbox_close (out);
		}
	    }
	  else if (config.action == PIPE)
	    {
	      outf = popen (pipecmd, "w");
	      if (outf == NULL)
		{
		  if (config.merr)
		    {
		      fprintf (stderr, "%s: %s: ", APPNAME, pipecmd);
		      perror (NULL);
		    }
		  exit (2);
		} /* if */
	      fprintf (outf, "%s\n%s", msg->headers, msg->body);
	      pclose (outf);
	    }
	  else if (config.action == COUNT)
	    ++count;
	  else if (config.action == DELETE && config.format == MBOX)
	    fprintf (tmpp, "%s\n%s", msg->headers, msg->body);
#ifdef HAVE_LIBZ
	  else if (config.action == DELETE && config.format == ZMBOX)
	    {
	      int quux, len, baz;

	      quux = 0;
	      baz = strlen (msg->headers);
	      for (;;)
		{
		  len = gzwrite (tmpp, (msg->headers+quux), 
			 	 (((quux + BUFLEN) < baz) ? BUFLEN : 
				  (baz - quux)));
		  quux += len;
		  if (quux == baz)
		    break;
		}
	      gzwrite(tmpp, "\n", 1);
	      quux = 0;
	      baz = strlen(msg->body);
	      for (;;)
		{
		  len = gzwrite(tmpp, (msg->body+quux), 
				(((quux + BUFLEN) < baz) ? BUFLEN : 
				 (baz - quux)));
		  quux += len;
		  if (quux == baz)
		    break;
		}
	    }
#endif /* HAVE_LIBZ */
	} /* if */
      else if (((((res1 == 0) | (res2 == 0)) ^ config.invert) && delete) &&
	       ((config.format == MH) || (config.format == NNMH) || 
		(config.format == NNML) || (config.format == MAILDIR)))
	m_unlink(msg->filename);

      free(msg->body);
      free(msg->headers);
      free(msg);
    } /* for */
  if ((config.format == MBOX) || (config.format == ZMBOX) ||
      (config.format == BZ2MBOX))
    mbox_close (mbox);
  else if ((config.format == MH) || (config.format == NNMH) ||
	   (config.format == NNML))
    mh_close(boxd);
}
/* }}} */

void recursive_scan (char path[])
     /* {{{  */

{
#ifdef HAVE_FTS_OPEN
  FTS *ftsfoo;
  FTSENT *ftsbar;
#endif /* HAVE_FTS_OPEN */

#ifdef HAVE_FTS_OPEN
  {
    char *p[2];

    p[0] = strdup (path);
    p[1] = 0;

    ftsfoo = fts_open (p, FTS_NOCHDIR, NULL);

    if (ftsfoo == NULL)
      {
	/* fixme (?) */
	perror(APPNAME);
	exit (2);
      }

    while ((ftsbar = fts_read (ftsfoo)))
      scan_mailbox (ftsbar->fts_path);

    fts_close (ftsfoo);
  }
#else
  ftw (path, (void *) scan_mailbox, 1);
#endif /* HAVE_FTS_OPEN */
}

/* }}} */

int md5_check_message (char *body, checksum_t *chksum)
     /* {{{  */
{
  struct md5_ctx a;
  unsigned char b[16];
  int i;

  md5_init_ctx (&a);
  if (body == NULL)
    md5_process_bytes ("", 0, &a);
  else
    md5_process_bytes (body, strlen(body), &a);
  md5_finish_ctx(&a, b);

  for (i = 0; i < chksum->n; i++)
    {
      if (0 == strncmp (chksum->md5[i], b, 16)) 
	return 1; 
    }

  chksum->md5 = 
	(char **) xrealloc (chksum->md5, (1 + chksum->n) * sizeof (char *));
  chksum->md5[chksum->n] = xstrdup (b);

  (chksum->n)++;

  return 0;
}
/* }}} */

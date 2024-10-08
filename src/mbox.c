/*
   mboxgrep - scan mailbox for messages matching a regular expression
   Copyright (C) 2000 - 2004, 2006, 2023 - 2024  Daniel Spiljar

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
*/

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
#  include <sys/file.h>
#endif /* HAVE_FLOCK */
#ifdef HAVE_LIBZ
#  include <zlib.h>
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
#  include <bzlib.h>
#endif /* HAVE_LIBBZ2 */
#define BUFLEN 16384

#include "mboxgrep.h"
#include "mbox.h"
#include "misc.h"
#include "wrap.h"

#ifdef HAVE_LIBDMALLOC
#  include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

mbox_t *
mbox_open (const char *path, const mbox_mode_t mbox_mode)
{
  mbox_t *mp;
  static int fd;
#ifndef HAVE_FLOCK
  struct flock lck;
#endif /* HAVE_FLOCK */

  mp = (mbox_t *) xmalloc (sizeof (mbox_t));
  mp->postmark_cache = NULL;

  if (0 == strcmp ("-", path))
    mp->fp = stdin;
  else
    {
      if (mbox_mode == w)
        fd = m_open (path, (O_WRONLY | O_CREAT | O_APPEND),
                     (S_IWUSR | S_IRUSR));
      else
        fd = m_open (path, O_RDONLY, 0);

      if (fd == -1)
        {
          errno = 0;
          return NULL;
        }

      if (config.lock > LOCK_NONE)
        mbox_lock (fd, path, mbox_mode);

      mp->fp = mbox_fdopen (fd, path, mbox_mode);
    }

  if (mbox_mode == r)
    {
      mp->postmark_cache = mbox_check_postmark (mp, path);

      if (! mp->postmark_cache)
        return NULL;
    }

  return mp;
}

void
mbox_close (mbox_t * mp)
{
  if (config.format == FORMAT_MBOX)
    fclose (mp->fp);
#ifdef HAVE_LIBZ
  else if (config.format == FORMAT_ZMBOX)
    gzclose (mp->fp);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  else if (config.format == FORMAT_BZ2MBOX)
    BZ2_bzclose (mp->fp);
#endif /* HAVE_LIBBZ2 */

  free (mp->postmark_cache);
  free (mp);
}

message_t *
mbox_read_message (mbox_t * mp)
{
  int isheaders = 1, s;
  char buffer[BUFSIZ];
  message_t *message;

  message = allocate_message ();

  s = strlen (mp->postmark_cache);
  message->headers =
    (char *) realloc (message->headers,
                      ((1 + s + message->hbytes) * sizeof (char)));
  strcpy (message->headers + message->hbytes, mp->postmark_cache);
  message->hbytes += s;

  for (;;)
    {
      if (config.format == FORMAT_MBOX)
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
      else if (config.format == FORMAT_ZMBOX)
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
      else if (config.format == FORMAT_BZ2MBOX)
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
        }
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
    }
  return NULL;
}


void
tmpmbox_create (const char *path)
{
  int foo;

  tmpfile_name (path);
  foo = tmpfile_create ();
  tmpfile_mod_own (foo, path);
}


void
tmpfile_name (const char *path)
{
  char *fname, *tmpdir;

  if (path == NULL)             /* no path prefix given, use /tmp or TMPDIR */
    {
      tmpdir = getenv ("TMPDIR");
      if (tmpdir == NULL)
        tmpdir = xstrdup ("/tmp");
      fname = xstrdup ("/mboxgrepXXXXXX");
    }
  else
    {
      tmpdir = xstrdup (path);
      fname = xstrdup (".XXXXXX");
    }

  config.tmpfilename =
    (char *) xmalloc ((strlen (tmpdir) + (strlen (fname) + 1))
                      * sizeof (char));
  sprintf (config.tmpfilename, "%s%s", tmpdir, fname);
}


void
mbox_write_message (message_t * msg, mbox_t * mbox)
{
  if (config.format == FORMAT_MBOX)
    fprintf (mbox->fp, "%s\n%s", msg->headers, msg->body);
#ifdef HAVE_LIBZ
  else if (config.format == FORMAT_ZMBOX)
    {
      gzwrite_loop (mbox->fp, msg->headers);
      gzwrite (mbox->fp, "\n", 1);
      gzwrite_loop (mbox->fp, msg->body);
    }
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  else if (config.format == FORMAT_BZ2MBOX)
    {
      bzwrite_loop (mbox->fp, msg->headers);
      BZ2_bzwrite (mbox->fp, "\n", 1);
      bzwrite_loop (mbox->fp, msg->body);
    }
#endif /* HAVE_LIBBZ2 */
}

void
tmpfile_mod_own (const int fd, const char *path)
{
  /* If we're root, copy {owner, group, perms} of mailbox to the tmpfile
   * so rename() will thus retain the original's ownership & permissions.
   */
  if (geteuid () == 0)
    {
      struct stat s;

      if (stat (path, &s) != -1)
        {
          if (fchown (fd, s.st_uid, s.st_gid) == -1)
            if (config.merr)
              perror (config.tmpfilename);
          if (fchmod (fd, s.st_mode) == -1)
            if (config.merr)
              perror (config.tmpfilename);
        }
      else if (config.merr)
        perror (path);
    }
}

int
tmpfile_create (void)
{
  int fd;

  fd = mkstemp (config.tmpfilename);
  if (-1 == fd)
    {
      if (config.merr)
        {
          fprintf (stderr, "%s: %s: ", APPNAME, config.tmpfilename);
          perror (NULL);
        }
      exit (2);
    }
  return fd;
}

void
mbox_lock (int fd, const char *path, const mbox_mode_t mbox_mode)
{
#ifdef HAVE_FLOCK
  int op;

  if (mbox_mode == r)
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
          exit (2);
        }
    }
}

void *
mbox_fdopen (int fd, const char *path, const mbox_mode_t mbox_mode)
{
  void *f;
  char *file_mode;

  if (mbox_mode == w)
    file_mode = xstrdup("wb");
  else
    file_mode = xstrdup("rb");

  if (config.format == FORMAT_MBOX)
    f = (FILE *) m_fdopen (fd, file_mode);
#ifdef HAVE_LIBZ
  else if (config.format == FORMAT_ZMBOX)
    f = (gzFile *) m_gzdopen (fd, file_mode);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  else if (config.format == FORMAT_BZ2MBOX)
    f = (BZFILE *) BZ2_bzdopen (fd, file_mode);
#endif /* HAVE_LIBBZ2 */

  if (f == NULL)
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

  return f;
}

char *
mbox_check_postmark(mbox_t *mp, const char *path)
{
  char *buffer;

  buffer = (char *) xmalloc (BUFSIZ * sizeof (char));
  memset (buffer, 0, BUFSIZ);

  if (config.format == FORMAT_MBOX)
    fgets (buffer, BUFSIZ, mp->fp);
#ifdef HAVE_LIBZ
  else if (config.format == FORMAT_ZMBOX)
    gzgets (mp->fp, buffer, BUFSIZ);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  else if (config.format == FORMAT_BZ2MBOX)
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
      if ((config.merr) && (buffer[0] != '\0'))
        {
          if (0 == strcmp ("-", path))
            fprintf (stderr, "%s: (standard input): Not an mbox folder\n",
                     APPNAME);
          else
            fprintf (stderr, "%s: %s: Not an mbox folder\n", APPNAME,
                     path);
        }
      if (config.format == FORMAT_MBOX)
        fclose (mp->fp);
#ifdef HAVE_LIBZ
      else if (config.format == FORMAT_ZMBOX)
        gzclose (mp->fp);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
      else if (config.format == FORMAT_BZ2MBOX)
        BZ2_bzclose (mp->fp);
#endif /* HAVE_LIBBZ2 */
      return NULL;
    }
  return buffer;
}

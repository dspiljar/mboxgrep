/* -*- C -*- 
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000 - 2003, 2023  Daniel Spiljar

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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
#include <bzlib.h>
#endif /* HAVE_LIBBZ2 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wrap.h"
#include "mboxgrep.h"

#ifndef APPNAME
#define APPNAME "mboxgrep"
#endif

#define BUFLEN 16384

int m_open (const char *pathname, int flags, mode_t mode)
{
  int blah;

  blah = open (pathname, flags, mode);
  if (blah == -1 && config.merr)
    {
      if (config.merr)
        {
          fprintf (stderr, "%s: %s: ", APPNAME, pathname);
          perror (NULL);
        }
      /* failure to open a file for writing should be fatal */
      if (((flags & O_WRONLY) == O_WRONLY) || ((flags & O_RDWR) == O_RDWR))
        exit (2);
    }
  return blah;
}      

FILE *m_fopen (const char *path, const char *mode)
{
  FILE *blah;

  blah = fopen (path, mode);
  if (blah == NULL)
    {
      if (config.merr)
        {
          fprintf (stderr, "%s: %s: ", APPNAME, path);
          perror (NULL);
        }
      if (mode[0] == 'w')
        exit (2);
    }
  return blah;
}

FILE *m_fdopen (int fildes, const char *mode)
{
  FILE *blah;

  blah = fdopen (fildes, mode);
  if (blah == NULL)
    {
      if (config.merr)
        perror (APPNAME);
    }
  return blah;
}

#ifdef HAVE_LIBZ

gzFile m_gzdopen (int fildes, const char *mode)
{
  gzFile blah;

  blah = gzdopen (fildes, mode);
  if (blah == NULL)
    {
      if (config.merr)
        perror (APPNAME);
    }
  return blah;
}

#endif /* HAVE_LIBZ */

DIR *m_opendir (const char *name)
{
  DIR *blah;

  blah = opendir (name);
  if (blah == NULL)
    {
      if (config.merr)
        {
          fprintf (stderr, "%s: %s: ", APPNAME, name);
          perror (NULL);
        }
    }
  return blah;
}

#ifndef HAVE_LIBDMALLOC

void *xmalloc (size_t size)
{
  void *foo;

  foo = malloc (size);
  if (foo == NULL)
    {
      if (config.merr)
        perror (APPNAME);
      exit (2);
    }
  return foo;
}

void *xrealloc (void *ptr, size_t size)
{
  void *foo;

  foo = realloc (ptr, size);
  if (foo == NULL)
    {
      if (config.merr)
        perror (APPNAME);
      exit (2);
    }
  return foo;
}

void *xcalloc (size_t nmemb, size_t size)
{
  void *foo;

  foo = calloc (nmemb, size);
  if (foo == NULL)
    {
      if (config.merr)
        perror (APPNAME);
      exit (2);
    }
  return foo;
}

char *xstrdup (const char *s)
{
  char *foo;

  foo = strdup (s);
  if (foo == NULL)
    {
      if (config.merr)
        perror (APPNAME);
      exit (2);
    }
  return foo;
}

#endif /* HAVE_LIBDMALLOC */

int m_unlink (const char *pathname)
{
  int baz;

  baz = unlink (pathname);
  if (baz == -1)
    {
      if (config.merr)
        {
          fprintf (stderr, "%s: %s: ", APPNAME, pathname);
          perror (NULL);
        }
    }
  return baz;
}

#ifdef HAVE_LIBZ

void gzwrite_loop (void *fp, char *str)
{
  int quux, len, baz;

  quux = 0;
  baz = strlen (str);
  for (;;)
    {
      len = gzwrite (fp, (str+quux), 
        (((quux + BUFLEN) < baz) ? BUFLEN : (baz - quux)));
      quux += len;
      if (quux == baz)
        break;
    }
}

#endif /* HAVE_LIBZ */

#ifdef HAVE_LIBBZ2

void bzwrite_loop (void *fp, char *str)
{
  int quux, len, baz;

  quux = 0;
  baz = strlen (str);
  for (;;)
    {
      len = BZ2_bzwrite (fp, (str+quux), 
        (((quux + BUFLEN) < baz) ? BUFLEN : (baz - quux)));
      quux += len;
      if (quux == baz)
        break;
    }
}

#endif /* HAVE_LIBBZ2 */

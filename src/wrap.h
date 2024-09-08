/*
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

/* This file is part of mboxgrep. */

/* wrappers around certain std functions */

#ifndef WRAP_H
#  define WRAP_H

#  include <config.h>
#  include <stdio.h>
#  include <sys/types.h>

#  ifdef HAVE_DIRENT_H
#    include <dirent.h>
#  else
#    ifdef HAVE_SYS_NDIR_H
#      include <sys/ndir.h>
#    endif
       /* HAVE_SYS_NDIR_H */
#    ifdef HAVE_SYS_DIR_H
#      include <sys/dir.h>
#    endif
       /* HAVE_SYS_DIR_H */
#    ifdef HAVE_NDIR_H
#      include <ndir.h>
#    endif
       /* HAVE_NDIR_H */
#  endif
       /* HAVE_DIRENT_H */
#  ifdef HAVE_LIBZ
#    include <zlib.h>
#  endif
       /* HAVE_LIBZ */

#  include <stdlib.h>

#  ifdef HAVE_LIBDMALLOC
#    include <dmalloc.h>
#  endif
       /* HAVE_LIBDMALLOC */

int m_open (const char *pathname, int flags, mode_t mode);

FILE *m_fopen (const char *path, const char *mode);
FILE *m_fdopen (int fildes, const char *mode);
#  ifdef HAVE_LIBZ
gzFile m_gzdopen (int fildes, const char *mode);
void gzwrite_loop (void *fp, char *str);
#  endif
       /* HAVE_LIBZ */
#  ifdef HAVE_LIBBZ2
void bzwrite_loop (void *fp, char *str);
#  endif
       /* HAVE_LIBBZ2 */

DIR *m_opendir (const char *name);

#  ifndef HAVE_LIBDMALLOC
void *xmalloc (size_t size);
void *xrealloc (void *ptr, size_t size);
void *xcalloc (size_t nmemb, size_t size);
char *xstrdup (const char *s);
#  endif
       /* HAVE_LIBDMALLOC */

int m_unlink (const char *pathname);

#if defined(__CYGWIN__) || defined(_WIN32)
#define NLSEP "\r\n"
#else
#define NLSEP "\n"
#endif 


#endif /* WRAP_H */

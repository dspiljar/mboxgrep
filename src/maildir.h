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

  $Id: maildir.h,v 1.6 2003-03-30 23:07:10 dspiljar Exp $ */

#ifndef MAILDIR_H
#define MAILDIR_H

#include <config.h>

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

#include "mboxgrep.h"

typedef struct
{
  DIR *new;
  DIR *cur;
} maildir_t;

maildir_t *maildir_open (const char *path);
int maildir_check (const char *path);
void maildir_create (const char *path);
void maildir_close (maildir_t *mdp);
message_t *maildir_read_message (maildir_t *mdp);
void maildir_write_message (message_t *m, const char *path);

#endif /* MAILDIR_H */

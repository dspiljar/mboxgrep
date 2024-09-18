/*
   mboxgrep - scan mailbox for messages matching a regular expression
   Copyright (C) 2000 - 2004, 2023 - 2024 Daniel Spiljar
   
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

#ifndef MBOX_H
#  define MBOX_H 1

#  include <config.h>

#  include "message.h"

typedef struct
{
  void *fp;
  void *fp_raw;
  char *postmark_cache;
} mbox_t;


mbox_t *mbox_open (const char *path, const char *mode);
void tmpmbox_create (const char *path);
void tmpfile_name (const char *path);
void tmpfile_mod_own (const int fd, const char *path);
int tmpfile_create (void);
void mbox_close (mbox_t * mbp);
message_t *mbox_read_message (mbox_t * mp);
void mbox_write_message (message_t * m, mbox_t * mbox);
void mbox_lock (int fd, const char *path, const char *mode);

#endif /* MBOX_H */

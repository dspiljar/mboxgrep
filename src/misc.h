/*
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000 - 2003, 2006, 2023  Daniel Spiljar

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

#ifndef MISC_H
#  define MISC_H 1

#  include "mboxgrep.h"
#  include "getopt.h"
#  include "message.h"
/* #include <time.h> */

void set_folder_format (const char *name);
void set_lock_method (const char *name);
/* time_t parse_date(char *datestr); */
char *parse_return_path (char *rpath);
void *allocate_message (void);
void postmark_print (message_t * msg);
void init_options (void);
void get_options (int *argc, char **argv,
                          struct option *long_options);
void check_options (void);
void set_option_action (action_t action, char *path);
void set_option_regextype (regextype_t regextype);

#define MESSAGE_ALLOC_BLOCK 0x10000

#endif /* MISC_H */

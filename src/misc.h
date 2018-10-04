/* -*- C -*- 
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000, 2001, 2002, 2003, 2006  Daniel Spiljar

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

  $Id: misc.h,v 1.9 2006-07-06 10:53:49 dspiljar Exp $ */

#ifndef MISC_H
#define MISC_H 1

#include "mboxgrep.h"
#include "getopt.h"
/* #include <time.h> */

format_t folder_format (const char *name);
lockmethod_t lock_method (const char *name);
/* time_t parse_date(char *datestr); */
char * parse_return_path(char *rpath);
void * malloc_message (void);
void postmark_print (message_t *msg);
void set_default_options (void);
void get_runtime_options (int *argc, char **argv, struct option *long_options);

#endif /* MISC_H */

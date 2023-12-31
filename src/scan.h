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

#ifndef SCAN_H
#  define SCAN_H 1

#  include "mboxgrep.h"

void scan_mailbox (char path[]);
void recursive_scan (char path[]);
int md5_check_message (char *body, checksum_t * chksum);

#endif /* SCAN_H */

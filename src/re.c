/*
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2006, 2023  Daniel Spiljar

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
#include <string.h>
#include <regex.h>
#ifdef HAVE_LIBPCRE2
#  define PCRE2_CODE_UNIT_WIDTH 32
#  include <pcre2.h>
#endif /* HAVE_LIBPCRE2 */
#include "mboxgrep.h"
#include "message.h"
#include "wrap.h"               /* xcalloc() et cetera */

#ifdef HAVE_LIBPCRE2
void
pcre_init (void)
{
  int errornumber;
  PCRE2_SIZE erroroffset;

  config.pcre_pattern =
    (pcre2_code *) pcre2_compile ((PCRE2_SPTR) config.regex_s, (PCRE2_SIZE) strlen (config.regex_s),
                                  (config.ignorecase ? PCRE2_CASELESS : 0),
                                  &errornumber, &erroroffset, NULL);
  if (config.pcre_pattern == NULL)
    {
      if (config.merr)
        {
          PCRE2_UCHAR buffer[256];

          pcre2_get_error_message (errornumber, buffer, sizeof(buffer));
          fprintf (stderr, "%s: PCRE2 compilation failed at offset %d: %s\n",
              APPNAME, (int) erroroffset, (char *) buffer);
        }
      exit (2);
    }

  config.match_data =
    (pcre2_match_data* ) pcre2_match_data_create_from_pattern (config.pcre_pattern, NULL);
}

void
pcre_match (message_t * msg)
{
  if (config.headers)
    config.res1 =
      pcre2_match ((pcre2_code *) config.pcre_pattern,
                   (PCRE2_SPTR) msg->headers, (int) strlen (msg->headers), 0, 0, config.match_data, NULL);
  if (config.body)
    config.res2 =
      pcre2_match ((pcre2_code *) config.pcre_pattern,
                   (PCRE2_SPTR) msg->body, (int) strlen (msg->body), 0, 0, config.match_data, NULL);

  config.res1 = config.res1 ^ 1;
  config.res2 = config.res2 ^ 1;
}
#endif /* HAVE_LIBPCRE2 */

void
regex_init (void)
{
  int flag1 = 0, flag2 = 0;
  int errcode = 0;
  char errbuf[BUFSIZ];

  if (config.ignorecase)
    flag1 = REG_ICASE;
  if (config.regextype == REGEX_EXTENDED)
    flag2 = REG_EXTENDED;

  config.posix_pattern = (regex_t *) xmalloc (sizeof (regex_t));
  errcode = regcomp ((regex_t *) config.posix_pattern, config.regex_s,
                     (flag1 | flag2 | REG_NEWLINE));
  if (0 != errcode)
    {
      if (config.merr)
        {
          regerror (errcode, (regex_t *) config.posix_pattern, errbuf,
                    BUFSIZ);
          fprintf (stderr, "%s: %s: %s\n", APPNAME, config.regex_s, errbuf);
        }
      exit (2);
    }
}

void
regex_match (message_t * msg)
{
  if (config.headers)
    config.res1 = regexec ((regex_t *) config.posix_pattern,
                           msg->headers, 0, NULL, 0);
  if (config.body)
    config.res2 = regexec ((regex_t *) config.posix_pattern,
                           msg->body, 0, NULL, 0);
}

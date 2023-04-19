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
#ifdef HAVE_LIBPCRE
#  include <pcre.h>
#endif /* HAVE_LIBPCRE */
#include "mboxgrep.h"
#include "message.h"
#include "wrap.h"               /* xcalloc() et cetera */

#ifdef HAVE_LIBPCRE
void
pcre_init (void)
{
  int errptr;
  const char *error;

  config.pcre_pattern =
    (pcre *) pcre_compile (config.regex_s,
                           (config.ignorecase ? PCRE_CASELESS : 0),
                           &error, &errptr, NULL);
  if (config.pcre_pattern == NULL)
    {
      if (config.merr)
        fprintf (stderr, "%s: %s: %s\n", APPNAME, config.regex_s, error);
      exit (2);
    }
}

void
pcre_match (message_t * msg)
{
  int of[BUFSIZ];

  if (config.headers)
    config.res1 =
      pcre_exec ((pcre *) config.pcre_pattern,
                 (pcre_extra *) config.pcre_hints,
                 msg->headers, (int) strlen (msg->headers), 0, 0, of, BUFSIZ);
  if (config.body)
    config.res2 =
      pcre_exec ((pcre *) config.pcre_pattern,
                 (pcre_extra *) config.pcre_hints,
                 msg->body, (int) strlen (msg->body), 0, 0, of, BUFSIZ);

  config.res1 = config.res1 ^ 1;
  config.res2 = config.res2 ^ 1;
}
#endif /* HAVE_LIBPCRE */

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

/*
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000 - 2004, 2006, 2023  Daniel Spiljar

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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifdef HAVE_LIBZ
#  include <zlib.h>
#endif /* HAVE_LIBZ */

#include "getopt.h"
#include "mboxgrep.h"
#include "misc.h"
#include "info.h"
#include "mbox.h"
#include "mh.h"
#include "scan.h"
#include "wrap.h"               /* xcalloc() et cetera */
#include "re.h"

#ifdef HAVE_LIBDMALLOC
#  include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

option_t config;
runtime_t runtime;

int
main (int argc, char **argv)
{
  int havemailbox = 0;
  int singlefile = 0;
  runtime.count = 0;
  runtime.maildir_count = 0;
  runtime.processed = 0;

  static struct option long_options[] = {
    {"count", 0, 0, 'c'},
    {"delete", 0, 0, 'd'},
    /*  {"date", 1, 0, 'D'}, */
    {"extended-regexp", 0, 0, 'E'},
    {"basic-regexp", 0, 0, 'G'},
    {"perl-regexp", 0, 0, 'P'},
    {"help", 0, 0, 'h'},
    {"ignore-case", 0, 0, 'i'},
    {"mailbox-format", 1, 0, 'm'},
    {"no", 1, 0, 'n'},
    {"pipe", 1, 0, 'p'},
    {"regexp", 1, 0, 'e'},
    {"invert-match", 0, 0, 'v'},
    {"version", 0, 0, 'V'},
    {"headers", 0, 0, 'H'},
    {"body", 0, 0, 'B'},
    {"no-messages", 0, 0, 's'},
    {"output", 1, 0, 'o'},
    {"no-duplicates", 0, 0, 200},
    {"no-file-lock", 0, 0, 201},
    {"debug", 0, 0, 202},
    {"file-lock", 1, 0, 'l'},
    {"recursive", 0, 0, 'r'},
    {"show-progress", 0, 0, 'x'},
    {"passthrough", 0, 0, 'Y'},
    {0, 0, 0, 0}
  };

  init_options ();

  get_options (&argc, argv, long_options);

  check_options ();

  runtime.cs = (checksum_t *) xmalloc (sizeof (checksum_t));
  runtime.cs->md5 = (char **) xcalloc (1, sizeof (char **));
  runtime.cs->n = 0;

  if (optind < argc && !config.haveregex)
    {
      config.regex_s = xstrdup (argv[optind]);
      config.haveregex = 1;
      ++optind;
    }

  if (config.haveregex)
    {
#ifdef HAVE_LIBPCRE2
      if (config.regextype == REGEX_PERL)
        pcre_init ();
      else
#endif /* HAVE_LIBPCRE2 */
        regex_init ();
    }
  else
    usage ();

  if (optind == (argc - 1))
    singlefile = 1;

  while (optind < argc)
    {
      if (config.action == ACTION_DELETE)
        {
          tmpmbox_create (argv[optind]);
          runtime.tmp_mbox = (mbox_t *) mbox_open (config.tmpfilename, "w");
        }

      config.boxname = xstrdup (argv[optind]);

      if (config.recursive)
        recursive_scan (argv[optind]);
      else
        scan_mailbox (argv[optind]);

      havemailbox = 1;

      if (config.action == ACTION_COUNT)
        {
          if (singlefile)
            fprintf (stdout, "%i\n", runtime.count);
          else
            {
              if (0 == strcmp ("-", argv[optind]))
                fprintf (stdout, "(standard input):%i\n", runtime.count);
              else
                fprintf (stdout, "%s:%i\n", argv[optind], runtime.count);
            }
        }
      if (config.action == ACTION_DELETE)
        {
          mbox_close (runtime.tmp_mbox);
          rename (config.tmpfilename, argv[optind]);
        }

      ++optind;
    }

  if (!havemailbox)
    {
      config.format = FORMAT_MBOX;
      scan_mailbox ("-");
      if (config.action == ACTION_COUNT)
        fprintf (stdout, "%i\n", runtime.count);
    }

  return 0;
}

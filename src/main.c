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

  $Id: main.c,v 1.32 2003/08/24 19:23:50 dspiljar Exp $ */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_LIBPCRE
#include <pcre.h>
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif /* HAVE_LIBZ */

#include "getopt.h"
#include "mboxgrep.h"
#include "misc.h"
#include "info.h"
#include "mbox.h"
#include "mh.h"
#include "scan.h"
#include "wrap.h" /* xcalloc() et cetera */

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

regex_t posix_pattern;
#ifdef HAVE_LIBPCRE
pcre *pcre_pattern;
pcre_extra *hints;
#endif /* HAVE_LIBPCRE */
char *boxname, *outboxname, *pipecmd, *tmpfilename;
int maildir_count = 0;
int count = 0;
void *tmpp;
checksum_t *cs;

int
main (int argc, char **argv)
     /* {{{  */

{
  int option_index = 0;
  int c;
#ifdef HAVE_LIBPCRE
  int errptr;
  const char *error;
#endif /* HAVE_LIBPCRE */
  int haveregex = 0, havemailbox = 0;
  static char *regex_s;
  int singlefile = 0;

  int errcode = 0;
  char errbuf[BUFSIZ];

  static struct option long_options[] = 
    /* {{{  */

    {
      {"count", 0, 0, 'c'},
      {"delete", 0, 0, 'd'},
      /*  {"date", 1, 0, 'D'}, */
      {"extended-regexp", 0, 0, 'E'},
      {"basic-regexp", 0, 0, 'G'},
      {"perl-regexp", 0, 0, 'P'},
      {"help", 0, 0, 'h'},
      {"ignore-case", 0, 0, 'i'},
      {"mailbox-format", 1, 0, 'm'},
      {"no", 1, 0, 'n' },
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
      {"file-lock", 1, 0, 'l'},
      {"recursive", 0, 0, 'r'},
      {0, 0, 0, 0}
    };

  /* }}} */

  config.perl = 0;
  config.extended = 1;
  config.invert = 0;
  config.headers = 0;
  config.body = 0;
  config.action = DISPLAY;
  config.dedup = 0;
  config.recursive = 0;
  config.ignorecase = 0;
  config.format = MBOX; /* default mailbox format */
  config.lock = FCNTL; /* default file locking method */
  config.merr = 1; /* report errors by default */

  while (1)
    {
      c = getopt_long (argc, argv, "BcdEe:GHhil:m:n:o:Pp:rsVv", long_options, 
		       &option_index);

      if (c == -1)
	break;

      switch (c)
	/* {{{  */

	{
	case '?':
	  usage();
	case 'c':
	  config.action = COUNT;
	  break;
	case 'd':
	  config.action = DELETE;
	  break;
	case 'e':
	  regex_s = xstrdup (optarg);
	  haveregex = 1;
	  break;
	case 'o':
	  outboxname = xstrdup (optarg);
	  config.action = WRITE;
	  break;
	case 'E':
	  config.extended = 1;
	  break;
	case 'G':
	  config.extended = 0;
	  break;
	case 'P':
#ifdef HAVE_LIBPCRE
	  config.extended = 0;
	  config.perl = 1;
#else
	  fprintf(stderr, 
		  "%s: Support for Perl regular expressions not "
		  "compiled in\n");
	  exit(2);
#endif /* HAVE_LIBPCRE */
	  break;
	case 'h':
	  help ();
	  break;
	case 'i':
	  config.ignorecase = 1;
	  break;
	case 'm':
	  config.format = folder_format (optarg);
	  break;
	case 'l':
	  config.lock = lock_method (optarg);
	  break;
	case 'p':
	  config.action = PIPE;
	  pipecmd = xstrdup (optarg);
	  break;
	case 'V':
	  version ();
	  break;
	case 'v':
	  config.invert = 1;
	  break;
	case 'H':
	  config.headers = 1;
	  break;
	case 'B':
	  config.body = 1;
	  break;
	case 's':
	  config.merr = 0;
	  break;
	case 201:
	  config.lock = 0;
	  break;
	case 'r':
	  config.recursive = 1;
	  break;
        case 200:
          config.dedup = 1;
          break;
        case 'n':
	  {
	    switch (optarg[0])
	      {
	        case 'd':
		  config.dedup = 1;
		  break;
		case 'l':
		  config.lock = 0;
		  break;
	        default:
		  fprintf(stderr, "%s: invalid option -- n%c\n", 
			  APPNAME, optarg[0]);
		  exit(2);
	      }
	  }
	} /* switch */

      /* }}} */
    } /* while */

  if ((config.body == 0) && (config.headers == 0))
    {
      config.body = 1;
      config.headers = 1;
    }

  if (config.format == MAILDIR && config.action == WRITE)
    {
      gethostname (config.hostname, HOST_NAME_SIZE);
      config.pid = (int) getpid ();
    }

  cs = (checksum_t *) xmalloc (sizeof (checksum_t));
  cs->md5 = (char **) xcalloc (1, sizeof (char **));
  cs->n = 0;

  if (optind < argc && ! haveregex)
    {
      regex_s = xstrdup (argv[optind]);
      haveregex = 1;
      ++optind;
    } /* if */

  if (haveregex) 
    {
#ifdef HAVE_LIBPCRE
      if (config.perl)
	/* {{{  */

	{
	  pcre_pattern = pcre_compile (regex_s, 
				       (config.ignorecase ? PCRE_CASELESS : 0),
				       &error, &errptr, NULL);
	  if (pcre_pattern == NULL)
	    {
	      if (config.merr)
		fprintf (stderr, "%s: %s: %s\n", APPNAME, regex_s, error);
	      exit(2);
	    }
	}

      /* }}} */
      else
#endif /* HAVE_LIBPCRE */
	/* {{{  */

	{
	  int flag1 = 0, flag2 = 0;
	  
	  if (config.ignorecase)
	    flag1 = REG_ICASE;
	  if (config.extended)
	    flag2 = REG_EXTENDED;
	
	  errcode = regcomp (&posix_pattern, regex_s, 
			     (flag1 | flag2 | REG_NEWLINE ));
	  if (0 != errcode)
	    {
	      if (config.merr)
		{
		  regerror (errcode, &posix_pattern, errbuf, BUFSIZ);
		  fprintf (stderr, "%s: %s: %s\n", APPNAME, regex_s, errbuf);
		}
	      exit (2);
	    } /* if */
	} /* if */

      /* }}} */
    } /* if */
  else
    usage ();

  if (optind == (argc - 1))
    singlefile = 1;

  while (optind < argc)
    /* {{{  */

    {
      if (config.action == DELETE) {
	  tmpp = tmpfile_open (argv[optind]);

	  /* If we're root, copy {owner, group, perms} of mailbox to the tmpfile
	   * so rename() will thus retain the original's ownership & permissions.
	   */
	  if (geteuid() == 0) {
	      struct stat s;
	      if (stat(argv[optind], &s) != -1) {
		  if (fchown(fileno(tmpp), s.st_uid, s.st_gid) == -1)
		      if (config.merr) perror(tmpfilename);
		  if (fchmod(fileno(tmpp), s.st_mode) == -1)
		      if (config.merr) perror(tmpfilename);
	      }
	      else if (config.merr) perror(argv[optind]);
	  }
      }

      boxname = xstrdup (argv[optind]);

      if (config.recursive)
	recursive_scan (argv[optind]);
      else
	scan_mailbox (argv[optind]);
      havemailbox = 1;
      if (config.action == COUNT)
	{
	  if (singlefile)
	    fprintf (stdout, "%i\n", count);
	  else
	    {
	      if (0 == strcmp ("-", argv[optind]))
		fprintf (stdout, "(standard input):%i\n", count);
	      else
		fprintf (stdout, "%s:%i\n", argv[optind], count);
	    }
	}
      if (config.action == DELETE)
	{
#ifdef HAVE_LIBZ
	  if (config.format == ZMBOX)
	    gzclose (tmpp);
#endif /* HAVE_LIBZ */
	  if (config.format == MBOX)
	    fclose (tmpp);
	  rename (tmpfilename, argv[optind]);
	}
      ++optind;
    } /* while */

  /* }}} */

  if (! havemailbox)
    /* {{{  */

    {
      config.format = MBOX;
      scan_mailbox ("-");
      if (config.action == COUNT)
	fprintf (stdout, "%i\n", count);
    }

  /* }}} */

  return 0;
} /* main */

/* }}} */

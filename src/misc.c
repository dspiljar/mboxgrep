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

#define _XOPEN_SOURCE  /* Pull in strptime(3) from time.h */
#define _BSD_SOURCE    /* Compensate for _XOPEN_SOURCE to pull in strdup(3)
                        * from string.h. */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "mboxgrep.h"
#include "wrap.h"
#include "getopt.h"
#include "info.h"
#include "message.h"

format_t
folder_format (const char *name)
{
  format_t f;

  if (0 == strncasecmp (name, "mbox", 4))
    f = MBOX;
  else if (0 == strncasecmp (name, "zmbox", 5))
    f = ZMBOX;
  else if (0 == strncasecmp (name, "gzmbox", 6))
    f = ZMBOX;
  else if (0 == strncasecmp (name, "bzmbox", 5))
    f = BZ2MBOX;
  else if (0 == strncasecmp (name, "bz2mbox", 5))
    f = BZ2MBOX;
  else if (0 == strncasecmp (name, "mh", 2))
    f = MH;
  else if (0 == strncasecmp (name, "nnml", 4))
    f = NNML;
  else if (0 == strncasecmp (name, "nnmh", 4))
    f = NNMH;
  else if (0 == strncasecmp (name, "maildir", 7))
    f = MAILDIR;
  else
    {
      if (config.merr)
        fprintf (stderr, "mboxgrep: %s: unknown folder type\n", name);
      exit (2);
    }

  return f;
}

lockmethod_t lock_method (const char *name)
{
  lockmethod_t l;

  if (0 == strncasecmp (name, "none", 4))
    l = NONE;
  else if (0 == strncasecmp (name, "off", 3))
    l = NONE;
#ifdef HAVE_FCNTL
  else if (0 == strncasecmp (name, "fcntl", 5))
    l = FCNTL;
#endif /* HAVE_FCNTL */
#ifdef HAVE_FLOCK
  else if (0 == strncasecmp (name, "flock", 5))
    l = FLOCK;
#endif /* HAVE_FLOCK */
  else
    {
      if (config.merr)
	fprintf (stderr, "mboxgrep: %s: unknown file locking method\n", name);
      exit (2);
    }

  return l;
}

/*
time_t parse_date(char *datestr)
{
  time_t t;
  const char *fmt = "%d%n%b%n%Y%n%T";
  int h, m;
  struct tm tm;
  char *str2, str1[BUFSIZ];

  sscanf (datestr, "Date: %[^\r\n]", str1);

  str2 = (char *) strptime (str1, "%d%n%b%n%Y%n%T", &tm);
  if (str2 == NULL)
    str2 = (char *) strptime (str1, "%a, %d%n%b%n%Y%n%T", &tm);
  if (str2 == NULL)
    return (time_t) 0;

  if (sscanf (str2, "%3d%2d", &h, &m) == 2)
    {
      tm.tm_hour -= h;
      tm.tm_min -= (h >= 0 ? m : -m);
      t = (time_t) mktime (&tm);
    }

   return t;
}
*/

char * parse_return_path(char *rpath)
{
  char *blah1, blah2[BUFSIZ];

  sscanf(rpath, "Return-Path: <%[^\r\n>]>", blah2);
  blah1 = xstrdup (blah2);

  return blah1;
}

void * allocate_message (void)
{
  message_t *message;

  message = (message_t *) xmalloc (sizeof (message_t));

  message->headers = (char *) xmalloc (sizeof (char));
  message->headers[0] = '\0';
  message->hbytes = 0;

  message->body = (char *) xmalloc (sizeof (char));
  message->body[0] = '\0';
  message->bbytes = 0;

  message->from = NULL;

  return message;
}

void postmark_print (message_t *msg)
{
  time_t tt;
  struct tm *ct;
  char date_str[80];

  tt = time (NULL);
  ct = localtime (&tt);
  strftime (date_str, 80, "%a %b %d %H:%M:%S %Y", ct);
  if (msg->from)
    fprintf (stdout, "From %s  %s\n", msg->from, date_str);
  else
    fprintf (stdout, "From nobody  %s\n", date_str);
}

void
set_default_options (void)
{
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
}

void
get_runtime_options (int *argc, char **argv, struct option *long_options)
{
  int option_index = 0, c;

  while (1)
    {
      c = getopt_long (*argc, argv, "BcdEe:GHhil:m:n:o:Pp:rsVv", long_options, 
		       &option_index);

      if (c == -1)
        break;

      switch (c)
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
            config.regex_s = xstrdup (optarg);
            config.haveregex = 1;
            break;
          case 'o':
            config.outboxname = xstrdup (optarg);
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
            config.pipecmd = xstrdup (optarg);
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
    } /* while */
}

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

#define _XOPEN_SOURCE           /* Pull in strptime(3) from time.h */
#define _DEFAULT_SOURCE         /* Compensate for _XOPEN_SOURCE to pull in strdup(3)
                                 * from string.h. */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "mboxgrep.h"
#include "misc.h"
#include "wrap.h"
#include "getopt.h"
#include "info.h"
#include "message.h"

/* Determine the folder format passed to -m. */

void
set_folder_format (const char *name)
{
  if (config.format > 0)
    {
      if (config.merr)
        fprintf (stderr, "%s: multiple mailbox types specified\n", APPNAME);
      exit (2);
    }

  if (0 == strncasecmp (name, "mbox", 4))
    config.format = FORMAT_MBOX;
  else if (0 == strncasecmp (name, "zmbox", 5))
    config.format = FORMAT_ZMBOX;
  else if (0 == strncasecmp (name, "gzmbox", 6))
    config.format = FORMAT_ZMBOX;
  else if (0 == strncasecmp (name, "bzmbox", 5))
    config.format = FORMAT_BZ2MBOX;
  else if (0 == strncasecmp (name, "bz2mbox", 5))
    config.format = FORMAT_BZ2MBOX;
  else if (0 == strncasecmp (name, "mh", 2))
    config.format = FORMAT_MH;
  else if (0 == strncasecmp (name, "nnml", 4))
    config.format = FORMAT_NNML;
  else if (0 == strncasecmp (name, "nnmh", 4))
    config.format = FORMAT_NNMH;
  else if (0 == strncasecmp (name, "maildir", 7))
    config.format = FORMAT_MAILDIR;
  else
    {
      if (config.merr)
        fprintf (stderr, "%s: %s: unknown folder type\n", APPNAME, name);
      exit (2);
    }
}

/* Determine the file locking method passed to -l. */

void
set_lock_method (const char *name)
{
  if (config.lock > 0)
    {
      if (config.merr)
        fprintf (stderr, "%s: conflicting file locking options specified\n", APPNAME);
      exit (2);
    }

  if (0 == strncasecmp (name, "none", 4))
    config.lock = LOCK_NONE;
  else if (0 == strncasecmp (name, "off", 3))
    config.lock = LOCK_NONE;
#ifdef HAVE_FCNTL
  else if (0 == strncasecmp (name, "fcntl", 5))
    config.lock = LOCK_FCNTL;
#endif /* HAVE_FCNTL */
#ifdef HAVE_FLOCK
  else if (0 == strncasecmp (name, "flock", 5))
    config.lock = LOCK_FLOCK;
#endif /* HAVE_FLOCK */
  else
    {
      if (config.merr)
        fprintf (stderr, "mboxgrep: %s: unknown file locking method\n", name);
      exit (2);
    }
}

/* Dead code */

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

char *
parse_return_path (char *rpath)
{
  char *blah1, blah2[BUFSIZ];

  sscanf (rpath, "Return-Path: <%[^\r\n>]>", blah2);
  blah1 = xstrdup (blah2);

  return blah1;
}

void *
allocate_message (void)
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

void
postmark_print (message_t * msg)
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

/* Initialize the option_t struct. */

void
init_options (void)
{
  config.regextype = REGEX_UNDEF;
  config.invert = 0;
  config.headers = 0;
  config.body = 0;
  config.action = ACTION_UNDEF;
  config.dedup = 0;
  config.recursive = 0;
  config.ignorecase = 0;
  config.format = FORMAT_UNDEF;
  config.lock = LOCK_UNDEF;     /* default file locking method */
  config.merr = 1;              /* report errors by default */
  config.debug = 0;
}

/* Parse command-line arguments and assign values to option_t. */

void
get_options (int *argc, char **argv, struct option *long_options)
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
          usage ();
        case 'c':
          set_option_action (ACTION_COUNT, NULL);
          break;
        case 'd':
          set_option_action (ACTION_DELETE, NULL);
          break;
        case 'e':
          config.regex_s = xstrdup (optarg);
          config.haveregex = 1;
          break;
        case 'o':
          set_option_action (ACTION_WRITE, optarg);
          break;
        case 'E':
          set_option_regextype (REGEX_EXTENDED);
          break;
        case 'G':
          set_option_regextype (REGEX_BASIC);
          break;
        case 'P':
          set_option_regextype (REGEX_PERL);
          break;
        case 'h':
          help ();
          break;
        case 'i':
          config.ignorecase = 1;
          break;
        case 'm':
          set_folder_format (optarg);
          break;
        case 'l':
          set_lock_method (optarg);
          break;
        case 'p':
          set_option_action (ACTION_PIPE, optarg);
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
        case 202:
          config.debug = 1;
          fprintf (stderr, "%s: %s, line %d: enable debugging\n",
                   APPNAME, __FILE__, __LINE__);
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
                set_lock_method ("none");
                break;
              default:
                fprintf (stderr, "%s: invalid option -- n%c\n",
                         APPNAME, optarg[0]);
                exit (2);
              }
          }
        }                       /* switch */
    }                           /* while */
}

/* Check the state of command-line options after parsing them.
 * Raise error on conflicting options and set uninitialized ones to default values.
 */

void
check_options (void)
{
  gethostname (config.hostname, HOST_NAME_SIZE);
  config.pid = (int) getpid ();

  if (config.action == ACTION_UNDEF)
    {
      config.action = ACTION_DISPLAY;
    }

  if (config.format == FORMAT_UNDEF)
    {
      config.format = FORMAT_MBOX;  /* default mailbox format */
    }

  if (config.regextype == REGEX_UNDEF)
    {
      config.regextype = REGEX_EXTENDED;  /* default regex type */
    }

  if ((config.body == 0) && (config.headers == 0))
    {
      config.body = 1;
      config.headers = 1;
    }
}

void
set_option_action (action_t action, char *path)
{
  if (config.action > 0)
    {
      if (config.merr)
        fprintf (stderr, "%s: conflicting actions specified\n", APPNAME);
      exit (2);
    }

  config.action = action;

  if (action == ACTION_WRITE)
    {
      config.outboxname = xstrdup (path);
    }

  if (action == ACTION_PIPE)
    {
      config.pipecmd = xstrdup (optarg);
    }
}

void
set_option_regextype (regextype_t regextype)
{
  if (config.regextype > 0)
    {
      if (config.merr)
        fprintf (stderr, "%s: conflicting matchers specified\n", APPNAME);
      exit (2);
    }

#ifndef HAVE_LIBPCRE2
  if (regextype == REGEX_PERL);
    {
      fprintf (stderr,
              "%s: Support for Perl regular expressions not compiled in\n",
              APPNAME);
      exit (2);
    }
#endif /* HAVE_LIBPCRE2 */
  config.regextype = regextype;
}

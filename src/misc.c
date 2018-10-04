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

  $Id: misc.c,v 1.13 2003/04/06 21:01:49 dspiljar Exp $ */

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

format_t
folder_format (const char *name)
     /* {{{  */
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
/* }}} */

lockmethod_t lock_method (const char *name)
     /* {{{  */
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
/* }}} */

/* time_t parse_date(char *datestr) */
     /* {{{  */
/* { */
/*   time_t t; */
/*   const char *fmt = "%d%n%b%n%Y%n%T"; */
/*   int h, m; */
/*   struct tm tm; */
/*   char *str2, str1[BUFSIZ]; */

/*   sscanf (datestr, "Date: %[^\r\n]", str1); */

/*   str2 = (char *) strptime (str1, "%d%n%b%n%Y%n%T", &tm); */
/*   if (str2 == NULL) */
/*     str2 = (char *) strptime (str1, "%a, %d%n%b%n%Y%n%T", &tm); */
/*   if (str2 == NULL) */
/*     return (time_t) 0; */
 
/*   if (sscanf (str2, "%3d%2d", &h, &m) == 2) */
/*     { */
/*       tm.tm_hour -= h; */
/*       tm.tm_min -= (h >= 0 ? m : -m); */
/*       t = (time_t) mktime (&tm); */
/*     } */

/*   return t; */
/* } */
/* }}} */

char * parse_return_path(char *rpath)
     /* {{{  */
{
  char *blah1, blah2[BUFSIZ];

  sscanf(rpath, "Return-Path: <%[^\r\n>]>", blah2);
  blah1 = xstrdup (blah2);

  return blah1;
}
/* }}} */

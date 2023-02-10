/*
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000 - 2004, 2006, 2010, 2023  Daniel Spiljar

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
#include <stdlib.h>

#include "mboxgrep.h"


void
print_wrap (char *str, int len, int *n)
{
  *n += len;
  fprintf (stdout, "%s", str);
  if (*n >= 50)
    {
      fprintf (stdout, "\n");
      *n = 0;
    }
  else fprintf (stdout, " ");
}

void
version (void)
{
  int n = 0;

  fprintf (stdout, "%s %s\n\n"
	   "Copyright (C) 2000 - 2004, 2006, 2010, 2023  Daniel Spiljar\n"
	   "This program is free software; you can redistribute it and/or "
	   "modify\nit under the terms of the GNU General Public License "
	   "as published by\nthe Free Software Foundation; either version "
	   "2 of the License, or\n(at your option) any later version.\n\n",
	   APPNAME, VERSION);
  fprintf (stdout, "Compilation options:\n");
#ifdef HAVE_DIRENT_H
  print_wrap ("HAVE_DIRENT_H", 13, &n);
#endif /* HAVE_DIRENT_H */
#ifdef HAVE_FCNTL
  print_wrap ("HAVE_FCNTL", 10, &n);
#endif /* HAVE_FCNTL */
#ifdef HAVE_FLOCK
  print_wrap ("HAVE_FLOCK", 10, &n);
#endif /* HAVE_FLOCK */
#ifdef HAVE_FTS_OPEN
  print_wrap ("HAVE_FTS_OPEN", 13, &n);
#else
# ifdef HAVE_FTW
  print_wrap ("HAVE_FTW", 8, &n);
# endif /* HAVE_FTW */
#endif /* HAVE_FTS_OPEN */
/*
  fprintf (stdout, "HAVE_LIBLOCKFILE ");
*/
#ifdef HAVE_LIBPCRE
  print_wrap ("HAVE_LIBPCRE", 12, &n);
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_LIBZ
  print_wrap ("HAVE_LIBZ", 9, &n);
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  print_wrap ("HAVE_LIBBZ2", 11, &n);
#endif /* HAVE_LIBBZ2 */
#ifdef HAVE_NDIR_H
  print_wrap ("HAVE_NDIR_H", 11, &n);
#endif /* HAVE_NDIR_H */
#ifdef HAVE_SYS_DIR_H
  print_wrap ("HAVE_SYS_DIR_H", 14, &n);
#endif /* HAVE_SYS_DIR_H */
#ifdef HAVE_SYS_NDIR_H
  print_wrap ("HAVE_SYS_NDIR_H", 15, &n);
#endif /* HAVE_SYS_NDIR_H */
#ifdef HAVE_STRPTIME
  print_wrap ("HAVE_STRPTIME", 15, &n);
#endif /* HAVE_STRPTIME */
#ifdef HAVE_REGCOMP
  print_wrap ("HAVE_REGCOMP", 15, &n);
#endif /* HAVE_REGCOMP */
#ifdef HAVE_LIBDMALLOC
  print_wrap ("HAVE_LIBDMALLOC", 15, &n);
#endif /* HAVE_LIBDMALLOC */
  fprintf (stdout, "\n");

  exit(0);
}

void 
help (void)
{
  fprintf(stdout, "%s %s - search MAILBOX for messages matching PATTERN\n\n",
	  APPNAME, VERSION);
  fprintf(stdout, 
	  "Miscellaneous:\n\n"
	  "  -h,  --help\t\t\tThis help screen\n"
	  "  -V,  --version\t\tDisplay version, copyright and\n"
	  "\t\t\t\tcompile-time options information\n"
	  "  -r,  --recursive\t\tDescend into directories recursively\n\n"
	  "Output control:\n\n"
	  "  -c,  --count\t\t\tPrint a count of matching messages\n"
	  "  -d,  --delete\t\t\tDelete matching messages\n"
          "  -nd, --no-duplicates\t\tIgnore duplicate messages\n"
	  "  -o,  --output=MAILBOX\t\tWrite messages to MAILBOX\n"
	  "  -p,  --pipe=COMMAND\t\tPipe each found message to COMMAND\n"
	  "  -s,  --no-messages\t\tSuppress most error messages\n\n"
	  "Matching criteria:\n\n"
	  "  -E,  --extended-regexp\tPATTERN is an extended regular expression\n"
	  "  -G,  --basic-regexp\t\tPATTERN is a basic regular expression\n");
#ifdef HAVE_LIBPCRE
  fprintf(stdout,  
	  "  -P,  --perl-regexp\t\tPATTERN is a Perl regular expression\n");
#endif /* HAVE_LIBPCRE */
  fprintf(stdout,
	  "  -e,  --regexp=PATTERN\t\tUse PATTERN as a regular expression\n"
	  "  -i,  --ignore-case\t\tIgnore case distinctions\n"
	  "  -v,  --invert-match\t\tSelect non-matching messages\n\n"
	  "Search scope selection:\n\n"
	  "  -H,  --headers\t\tMatch PATTERN against message headers\n"
	  "  -B,  --body\t\t\tMatch PATTERN against message body\n\n"
	  "File locking:\n\n"
	  "  -nl, --no-file-lock\t\tDo not lock files\n"
	  "  -l,  --file-lock=METHOD\tSelect file locking METHOD\n"
	  "\t\t\t\tMETHOD is");
#ifdef HAVE_FCNTL
  fprintf(stdout, " `fcntl',");
#endif /* HAVE_FCNTL */
#ifdef HAVE_FLOCK
  fprintf(stdout, " `flock',");
#endif /* HAVE_FLOCK */
  fprintf(stdout, " or `none'\n\n"
	  "Mailbox type selection:\n\n"
	  "  -m,  --mailbox-format=TYPE\tSelect mailbox TYPE\n"
	  "\t\t\t\tTYPE is `mbox', ");
#ifdef HAVE_LIBZ
  fprintf(stdout, "`zmbox', ");
#endif /* HAVE_LIBZ */
#ifdef HAVE_LIBBZ2
  fprintf(stdout, "`bz2mbox', ");
#endif /* HAVE_LIBBZ2 */
  fprintf(stdout,
	  "`mh',\n"
	  "\t\t\t\t`nnml', `nnmh', or `maildir'.\n\n"
	  "Mail bug reports and flames to <%s>.\n", BUGREPORT_ADDR);

  exit(0);
}

void 
usage (void)
{
  printf ("Usage: %s [OPTION] PATTERN MAILBOX ...\n\n"
          "Try `%s --help' for more information.\n", APPNAME, APPNAME);

  exit (2);
}

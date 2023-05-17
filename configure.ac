#  mboxgrep - scan mailbox for messages matching a regular expression
#  Copyright (C) 2000 - 2003, 2023  Daniel Spiljar
#
#  Mboxgrep is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Mboxgrep is distributed in the hope that it will be useful, but 
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with mboxgrep; if not, write to the Free Software Foundation, 
#  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Process this file with autoconf to produce a configure script.

# Yawn.
AC_INIT([mboxgrep], [0.7.11], [dspiljar@datatipp.se], [mboxgrep], [https://www.mboxgrep.org/])
#AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([foreign])
AC_LANG([C])
AC_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for standard (sp?) header files
AC_HEADER_STDC
AC_HEADER_DIRENT

# Checks for libraries.

# Check for PCRE2 library
AC_ARG_WITH(pcre2, [  --without-pcre2          Compile without Perl regexp support],,
[
	AC_PATH_PROG(PCRE2_CONFIG, pcre2-config)

	if test "$PCRE2_CONFIG"; then
	   CFLAGS="$CFLAGS `$PCRE2_CONFIG --cflags`"
	   LIBS="$LIBS `$PCRE2_CONFIG --libs32`"

	   AC_LINK_IFELSE(
	   [
#define PCRE2_CODE_UNIT_WIDTH 32
#include <pcre2.h>

int main ()
{
  return 0;
}
	   ],
	   AC_DEFINE(HAVE_LIBPCRE2),
	   [
		AC_MSG_NOTICE(found pcre2-config but could not compile test program.)
		AC_MSG_FAILURE(is PCRE2 properly installed?)
	   ]
)
	else
	   AC_MSG_NOTICE(pcre2-config not found)
	   AC_MSG_NOTICE(trying to find PCRE2 anyway)
	   AC_CHECK_LIB(pcre2, main)
	fi
]
)

# Check for zlib
AC_ARG_WITH(z, [  --without-zlib          Compile without suport for gzip compressed mailboxes],,
        AC_CHECK_LIB(z, gzopen))

# Check for bzlib
AC_ARG_WITH(z, [  --without-bzlib         Compile without suport for bzip2 compressed mailboxes],,
        AC_CHECK_LIB(bz2, BZ2_bzopen))

# Check for dmalloc
AC_ARG_WITH(dmalloc, [  --with-dmalloc          Compile with dmalloc library],
	AC_CHECK_LIB(dmalloc, main))

# Check for our favorite sarcastic cat's sanity.  8)
AC_ARG_WITH(sense-of-humor, [  --without-sense-of-humor  If you have no sense of humor],,
        AC_CHECK_LIB(garfield, like_monday))

# AC_ARG_WITH(lockfile, [  --without-lockfile  Blah],,
# 	AC_CHECK_LIB(lockfile, maillock))

AC_CHECK_FUNCS(fcntl)
AC_CHECK_FUNCS([flock])
AC_CHECK_FUNCS([regcomp])
AC_CHECK_FUNCS([strptime])
AC_CHECK_FUNCS(ftw)
AC_CHECK_FUNCS(fts_open)

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 src/Makefile])
AC_OUTPUT

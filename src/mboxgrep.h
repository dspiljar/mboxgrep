/*
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000 - 2003, 2006, 2023  Daniel Spiljar

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

#ifndef MBOXGREP_H
#define MBOXGREP_H

#define APPNAME "mboxgrep"
#define VERSION "0.7.11"
#define BUGREPORT_ADDR "dspiljar AT datatipp.se"

#define HOST_NAME_SIZE 256

#include <config.h>

#include <time.h>  /* for tm structure */

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif /* HAVE_SYS_NDIR_H */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* HAVE_SYS_DIR_H */
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif /* HAVE_NDIR_H */
#endif /* HAVE_DIRENT_H */

typedef enum
  {
    FORMAT_UNDEF,
    FORMAT_MBOX,
    FORMAT_ZMBOX,
    FORMAT_MH,
    FORMAT_NNML,
    FORMAT_NNMH,
    FORMAT_MAILDIR,
    FORMAT_BZ2MBOX
  }
format_t;

typedef enum
  {
    LOCK_UNDEF,
    LOCK_NONE,
    LOCK_FCNTL,
    LOCK_FLOCK
  }
lockmethod_t;

typedef enum
  {
    ACTION_UNDEF,
    ACTION_DISPLAY,
    ACTION_WRITE,
    ACTION_COUNT,
    ACTION_DELETE,
    ACTION_PIPE
  }
action_t;

typedef enum
  {
    REGEX_UNDEF,
    REGEX_BASIC,
    REGEX_EXTENDED,
    REGEX_PERL
  }
regextype_t;

typedef struct
{
  FILE *fp;
  DIR *dp;
  DIR *newp;
  DIR *curp;
  char *path;
}
folder_t;

typedef struct
{
  char **md5;
  int n;
}
checksum_t;

typedef struct
{
  int body;
  int headers;
  int dedup;
  int recursive;
  int invert;
  int count;
  int ignorecase;
  int merr;
  int pid;
  int haveregex;
  int debug;

  char hostname[HOST_NAME_SIZE];
  char *boxname, *outboxname, *pipecmd, *tmpfilename, *regex_s;

  void *pcre_pattern, *pcre_hints, *posix_pattern;
  int res1, res2;

  action_t action;
  format_t format;
  lockmethod_t lock;
  regextype_t regextype;
}
option_t;

typedef struct
{
  int count;
  int maildir_count;
  checksum_t *cs;
  /* mbox_t *tmp_mbox; */
  void *tmp_mbox;
}
runtime_t;

extern option_t config;
extern runtime_t runtime;

#endif /* MBOXGREP_H */

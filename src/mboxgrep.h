/* -*- C -*- 
  mboxgrep - scan mailbox for messages matching a regular expression
  Copyright (C) 2000, 2001, 2002, 2003, 2006  Daniel Spiljar

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

  $Id: mboxgrep.h,v 1.31 2006-10-22 23:34:49 dspiljar Exp $ */

#ifndef MBOXGREP_H
#define MBOXGREP_H

#define APPNAME "mboxgrep"
#define VERSION "0.7.10"
#define BUGREPORT_ADDR "dspiljar@datatipp.se"

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

#include "mbox.h"

typedef enum
  {
    MBOX,
    ZMBOX,
    MH,
    NNML,
    NNMH,
    MAILDIR,
    BZ2MBOX
  }
format_t;

typedef enum
  {
    NONE,
    FCNTL,
    FLOCK
  }
lockmethod_t;

typedef enum
  {
    DISPLAY,
    WRITE,
    COUNT,
    DELETE,
    PIPE
  }
action_t;

typedef struct
{
  char *filename; /* used with directory formats, such as maildir or MH */
  char *msgid; 
  char *from;
  char *headers;
  int hbytes;
  char *body;
  int bbytes;
  time_t date;
}
message_t;

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
  int extended;
  int perl;
  
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

  char hostname[HOST_NAME_SIZE];
  char *boxname, *outboxname, *pipecmd, *tmpfilename, *regex_s;

  void *pcre_pattern, *pcre_hints, *posix_pattern;
  int res1, res2;

  action_t action;
  format_t format;
  lockmethod_t lock;
}
  option_t;

typedef struct
{
  int count;
  int maildir_count;
  checksum_t *cs;
  mbox_t *tmp_mbox;
}
  runtime_t;

option_t config;
runtime_t runtime;

#endif /* MBOXGREP_H */

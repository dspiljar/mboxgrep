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

  $Id: mh.c,v 1.18 2006-02-20 17:12:22 dspiljar Exp $ */

#include <stdio.h>
#include <string.h>

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
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

#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "mboxgrep.h"
#include "misc.h"
#include "mh.h"
#include "wrap.h"

#ifdef HAVE_LIBDMALLOC
# include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

extern option_t config;

DIR *mh_open (const char *path)
{
  DIR *dp;

  dp = opendir (path);
  if (dp == NULL)
    {
      if (config.merr)
	{
	  fprintf (stderr, "%s: %s: ", APPNAME, path);
	  perror (NULL);
	}
      errno = 0;
      return NULL;
    }
  return dp;
} /* mh_open */

void mh_close (DIR *dp)
{
  closedir (dp);
} /* mh_close */

message_t *mh_read_message (DIR *dp)
{
  int isheaders = 1;
  int have_from = 0, have_date = 0, have_sender = 0;
  static int s;
  message_t *message;
  struct dirent *d_content;
  char buffer[BUFSIZ], *filename;
  FILE *fp;

  message = allocate_message ();

  filename = NULL;

  for(;;)
    {
      d_content = readdir(dp);
      if (d_content == NULL) return NULL;
      if (d_content->d_name[0] == '.')
	continue;

      filename = (char *) xrealloc 
	(filename, ((strlen (d_content->d_name)) + 
		    (strlen (config.boxname)) + 2));

/*       message->headers = (char *) xrealloc (message->headers, 0); */
/*       message->hbytes = 0; */
/*       message->body = (char *) xrealloc (message->body, 0); */
/*       message->bbytes = 0; */

      filename[0] = '\0';
      sprintf (filename, "%s/%s", config.boxname, d_content->d_name);
      fp = m_fopen (filename, "r");
      isheaders = 1;
      if (fp == NULL)
	{
	  free (message->headers);
	  free (message->body);
	  message->hbytes = 0;
	  message->bbytes = 0;
	  continue;
	}

      fgets (buffer, BUFSIZ, fp);

/*       if (config.format == NNML || config.format == NNMH) */
/* 	{ */
/* 	  if (0 != strncmp ("X-From-Line: ", buffer, 13)) */
/* 	    { */
/* 	      if (config.merr) */
/* 		fprintf (stderr, "%s: %s: Not a Gnus folder message\n",  */
/* 			 APPNAME, filename); */
/* 	      fclose (fp); */
/* 	      free (message->headers); */
/* 	      free (message->body); */
/* 	      message->hbytes = 0; */
/* 	      message->bbytes = 0; */
/* 	      continue; */
/* 	    } */
/* 	} */
      
      fseek (fp, 0, SEEK_SET);

      while (fgets (buffer, BUFSIZ, fp) != NULL)
	{
	  s = strlen (buffer);
	  if (0 == strncmp ("\n", buffer, 1) && isheaders == 1)
	    {
	      isheaders = 0;
	      continue;
	    } /* if */
	  if (isheaders)
	    {
	      if (0 == strncasecmp ("From: ", buffer, 6))
		have_from = 1;
	      if (0 == strncasecmp ("Sender: ", buffer, 8))
		have_sender = 1;
	      if (0 == strncasecmp ("Date: ", buffer, 6))
		have_date = 1;
	      if (0 == strncasecmp ("Return-Path: ", buffer, 13))
		  message->from = parse_return_path (buffer);

	      message->headers =
		(char *) realloc (message->headers,
				  ((1 + s + message->hbytes) * sizeof (char)));
	      strcpy (message->headers + message->hbytes, buffer);
	      message->hbytes += s;
	    } /* if */
	  else
	    {
	      message->body =
		(char *) realloc (message->body,
				  ((1 + s + message->bbytes) * sizeof (char)));
	      strcpy (message->body + message->bbytes, buffer);
	      message->bbytes += s;
	    } /* else */
	} /* while */

      if ((!have_from && !have_sender)|| !have_date)
	{
	  if (config.merr)
	    fprintf (stderr, "%s: %s: Not a RFC 2822 message\n",
		     APPNAME, filename);
	  fclose (fp);
	  free (message->headers);
	  message->headers = NULL;
	  free (message->body);
	  message->body = NULL;
	  message->hbytes = 0;
	  message->bbytes = 0;
	  continue;
	}

      else
	{
	  message->filename = (char *) xstrdup (filename);
	  fclose (fp);
	  free (filename);

	  return message;
	}
    } /* for */
} /* mh_read_message */

void mh_write_message (message_t *m, const char *path)
{
  struct dirent *dc;
  int x, y = 0;
  char s1[BUFSIZ];
  DIR *d; FILE *f;

  d = m_opendir (path);
  rewinddir (d);

  while ((dc = readdir (d)) != NULL)
    {
      x = strtol (dc->d_name, NULL, 10);
      if (x > y)
	y = x;
    }
  y++;
  sprintf (s1, "%s/%i", path, y);
  
  f = m_fopen (s1, "w");
  fprintf (f, "%s\n%s", m->headers, m->body);
  fclose (f);
}

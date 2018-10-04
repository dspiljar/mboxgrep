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

  $Id: maildir.c,v 1.25 2006-10-19 01:53:43 dspiljar Exp $ */

#include <config.h>

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

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mboxgrep.h"
#include "misc.h"
#include "maildir.h"
#include "wrap.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif /* HAVE_LIBDMALLOC */

maildir_t *
maildir_open (const char *path)
{
  static maildir_t *mp;
  char *dirname;
  DIR *foo;

  foo = m_opendir (path);
  if (foo == NULL)
      return NULL;

  closedir (foo);

  if (-1 == maildir_check (path))
    {
      if (config.merr)
	  fprintf (stderr, "%s: %s: Not a maildir folder\n", APPNAME, path);
      return NULL;
    }

  dirname = (char *) xmalloc((sizeof (char) * (strlen (path) + 5)));

  mp = (maildir_t *) xmalloc(sizeof (maildir_t));
  sprintf (dirname, "%s/new", path);
  mp->new = m_opendir (dirname);
  if (mp->new == NULL)
    {
      if (config.merr)
	{
	  fprintf (stderr, "%s: %s: ", APPNAME, dirname);
	  perror (NULL);
	}
      free (dirname);
      errno = 0;
      return NULL;
    }

  sprintf (dirname, "%s/cur", path);
  mp->cur = m_opendir (dirname);
  if (mp->cur == NULL)
    {
      if (config.merr)
	{
	  fprintf (stderr, "%s: %s: ", APPNAME, dirname);
	  perror (NULL);
	}
      free (dirname);
      errno = 0;
      return NULL;
    }  

  free (dirname);
  return mp;
} /* maildir_open */

message_t *
maildir_read_message (maildir_t *mdp)
{
  int isheaders = 1;
  int have_from = 0, have_to = 0, have_message_id = 0, have_sender = 0,
      have_date = 0;
  static message_t *message;
  static struct dirent *d_content;
  char *filename, buffer[BUFSIZ];
  static FILE *fp;
  static int s;

  message = malloc_message ();

  for(;;)
    {
      if (mdp->new != NULL) 
	{
	  d_content = readdir (mdp->new);
	  if (d_content == NULL)
	    {
	      closedir (mdp->new);
	      mdp->new = NULL;
	    }
	}
      if (mdp->new == NULL && mdp->cur != NULL)
	{
	  d_content = readdir (mdp->cur);
	  if (d_content == NULL)
	    {
	      closedir (mdp->cur);
	      mdp->cur = NULL;
	    }
	}
      if (mdp->new == NULL && mdp->cur == NULL)
{
	free (message->headers);
	free (message->body);
	free (message);
	return NULL;
}

      if (d_content->d_name[0] == '.')
	continue;

      filename = 
	(char *) xmalloc ((sizeof (char)*((strlen (d_content->d_name)) 
					  + (strlen (config.boxname)) + 6)));

/*
      filename = 
	(char *) alloca((sizeof(char)*((strlen(d_content->d_name))
					 + (strlen(config.boxname)) + 2)));
*/
      if (mdp->new != NULL)
	sprintf (filename, "%s/new/%s", config.boxname, d_content->d_name);
      else
	sprintf (filename, "%s/cur/%s", config.boxname, d_content->d_name);
      message->filename = (char *) xstrdup (filename);
      free (filename);

      isheaders = 1;
      fp = m_fopen(message->filename, "r");

      if (fp == NULL)
	  continue;

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
	      if (0 == strncasecmp ("To: ", buffer, 4))
		have_to = 1;
	      if (0 == strncasecmp ("Message-ID: ", buffer, 12))
		have_message_id = 1;
	      if (0 == strncasecmp ("Return-Path: ", buffer, 13))
		  message->from = parse_return_path(buffer);

	      message->headers =
		(char *) xrealloc (message->headers,
				  ((1 + s + message->hbytes) * sizeof (char)));
	      strcpy (message->headers + message->hbytes, buffer);
	      message->hbytes += s;
	    } /* if */
	  else
	    {
	      message->body =
		(char *) xrealloc (message->body,
				  ((1 + s + message->bbytes) * sizeof (char)));
	      strcpy (message->body + message->bbytes, buffer);
	      message->bbytes += s;
	    } /* else */
	} /* while */
/*       if (!have_from || !have_to || !have_message_id) */
      if ((!have_from && !have_sender)|| !have_date)
	{
	  if (config.merr)
	    fprintf(stderr, "%s: %s: Not a RFC 2822 message\n", 
		    APPNAME, message->filename);
	  fclose(fp);
	  continue;
	}
      
      fclose(fp);
      return message; 
    } /* for */
} /* maildir_read_message */

void 
maildir_write_message (message_t *m, const char *path)
{
  char bla[BUFSIZ], *s1, *s2;
  int t;
  static FILE *f1;

  runtime.maildir_count++;
  t = (int) time (NULL);

  sprintf (bla, "%i.%i_%i.%s",
	   t, config.pid, runtime.maildir_count, config.hostname);
  s1 = (char *) xmalloc ((strlen (path) + strlen (bla) + 6) * sizeof (char));
  sprintf(s1, "%s/tmp/%s", path, bla);
  s2 = (char *) xmalloc ((strlen (path) + strlen (bla) + 6) * sizeof (char));
  sprintf(s2, "%s/new/%s", path, bla);

  f1 = m_fopen (s1, "w");
  fprintf (f1, "%s\n%s", m->headers, m->body);
  fclose (f1);
  rename (s1, s2);
}

int 
maildir_check (const char *path)
{
  static struct stat fs;
  static int i;
  char *s;

  s = (char *) xmalloc ((strlen (path) + 5) * sizeof (char));

  sprintf (s, "%s/cur", path);
  i = stat (s, &fs);
  if (-1 == i) return -1;
  if (! S_ISDIR (fs.st_mode)) return -1;

  sprintf (s, "%s/new", path);
  i = stat (s, &fs);
  if (-1 == i) return -1;
  if (! S_ISDIR (fs.st_mode)) return -1;

  sprintf(s, "%s/tmp", path);
  i = stat (s, &fs);
  if (-1 == i) return -1;
  if (! S_ISDIR (fs.st_mode)) return -1;

  free (s);

  return 0;
}

void 
maildir_create (const char *path)
{
  char *s;
  int i;

  s = (char *) xmalloc ((strlen (path) + 4) * sizeof(char));
  errno = 0;

  for (;;)
    {
      sprintf(s, "%s", path);
      i = mkdir (s, S_IRWXU);
      if (-1 == i) 
	break;
      sprintf(s, "%s/new", path);
      i = mkdir (s, S_IRWXU);
      if (-1 == i)
	break;
      sprintf(s, "%s/cur", path);
      i = mkdir (s, S_IRWXU);
      if (-1 == i)
	break;
      sprintf(s, "%s/tmp", path);
      i = mkdir (s, S_IRWXU);
      if (-1 == i)
	break;

      break;
    }

  if (errno != 0)
    {
      if (config.merr)
	{
	  fprintf(stderr, "%s:%s: ", APPNAME, s);
	  perror (NULL);
	}
    }
}

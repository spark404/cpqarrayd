/*
   CpqArray Deamon, a program to monitor and remotely configure a 
   SmartArray controller.
   Copyright (C) 1999  Hugo Trippaers

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
   $Header$
 */

#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ida_ioctl.h>

#include "cpqarrayd.h"

extern char *optarg;
extern int optind, opterr, optopt;

#define DEBUG(x)  fprintf(stderr, x)

void print_usage() 
{
  printf("cpqarrayd [options]\n");
  printf("   -h      prints this text\n");
  printf("   -d      enables debugging\n");
  printf("           disables forking to the background\n");
  printf("   -v      gives more ouput\n");
}


/* true when at least one controller is found
 * false otherwise
 */
int check4controllers(struct opts opts) 
{
  int devicefd;
  ida_ioctl_t io;
  int found = 0;
 
  /* does this device exist ? */
  if ((access ("/dev/ida/c0d0", R_OK | F_OK)) == 0)
    {
      /* ok, now check if something is listing to ioctls */
      devicefd = open ("/dev/ida/c0d0", O_RDONLY);
      memset (&io, 0, sizeof (io));
      io.cmd = ID_CTLR;
      if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
	{
	  if (opts.debug) perror("ioctl");
	}
      else
	{
	  found = 1; /* first controller found, no need to check others */
	}
    }
  return found;
}


int main(int argc, char *argv[]) 
{
  char option;
  struct opts opts; /* commandline options */
  
  /* check options */
  while ((option = getopt (argc, argv, "dvh")) != EOF)
    {
      switch (option)
        {
        case 'v':
          opts.verbose = 1;
          break;
	case 'd':
	  opts.debug = 1;
	  break;
	case '?':
	case 'h':
	  print_usage();
	  exit(0);
	  break;
	default:
	  fprintf (stderr, "How did you end up here??\n");
	}
    }
  
  /* Check for existance of array controllers */
  printf("Checking for controllers.. ");
  if (! check4controllers(opts)) {
    printf("None Found!\n");
    fprintf(stderr, "  You don't seem to have any controllers\n");
    fprintf(stderr, "  therefore it's rather senseless for me to run.\n");
    exit(1);
  }
  else {
    printf("Ok\n");
  }

  return 0;   /* should return something */
}


    



	  
	  

		  
	      
	      
    



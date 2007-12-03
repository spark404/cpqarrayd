/*
   CpqArray Deamon, a program to monitor and remotely configure a 
   SmartArray controller.
   Copyright (C) 1999-2003  Hugo Trippaers

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

#include "config.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#if defined(HAVE_LINUX_COMPILER_H)
  #include <linux/compiler.h>
#endif

#if defined(__linux__)
  #include <ida_ioctl.h>
#endif

#if defined(__freebsd__)
  #include <idavar.h>
#endif

#include <signal.h>
#include <syslog.h>
#include <netdb.h>

#include <errno.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#include "cpqarrayd.h"
#include "discover.h"
#include "status.h"

const char *controllers[] =
{
  "/dev/ida/c0d0",
  "/dev/ida/c1d0",
  "/dev/ida/c2d0",
  "/dev/ida/c3d0",
  "/dev/ida/c4d0",
  "/dev/ida/c5d0",
  "/dev/ida/c6d0",
  "/dev/ida/c7d0"
};

const char *statusstr[] = {
        "Logical drive /dev/ida/c%dd%d ok",
        "Logical drive /dev/ida/c%dd%d failed",
        "Logical drive /dev/ida/c%dd%d not configured",
        "Logical drive /dev/ida/c%dd%d using interim recovery mode, %3.2f%% done",
        "Logical drive /dev/ida/c%dd%d ready for recovery operation",
        "Logical drive /dev/ida/c%dd%d is currently recovering, %3.2f%% done",
        "Wrong physical drive was replaced",
        "A physical drive is not properly connected",
        "Hardware is overheating",
        "Hardware has overheated",
        "Logical drive /dev/ida/c%dd%d is currently expanding, %3.2f%% done",
        "Logical drive /dev/ida/c%dd%d is not yet available",
        "Logical drive /dev/ida/c%dd%d is queued for expansion",
};

extern char *optarg;
extern int optind, opterr, optopt;

int ctrls_found_num;
struct controller ctrls_found[MAX_CTRLS];

unsigned int myip;

int keeprunning = 1;


#define DEBUG(x)  fprintf(stderr, x)

void print_usage() 
{
  printf("cpqarrayd [options]\n");
  printf("   -h         prints this text\n");
  printf("   -d         enables debugging\n");
  printf("              disables forking to the background\n");
  printf("   -v         gives more output\n");
  printf("   -f         don't fork\n");
  printf("   -t <host>  trap destination\n");
}


void signal_handler(int signal) 
{
  keeprunning = 0;
}


int main(int argc, char *argv[]) 
{
  char option;
  struct opts opts; /* commandline options */
  int result, i;
  FILE *pidfile;
  struct sigaction myhandler;
  char buffer[1024];
  struct hostent *myhost;
  struct utsname *myhostname;
  
  memset(&opts, 0, sizeof(struct opts));
  
  /* check options */
  while ((option = getopt (argc, argv, "dfvhst:")) != EOF)
    {
      switch (option)
        {
        case 'v':
          opts.verbose = 1;
          break;
	case 'd':
	  opts.debug = 1;
	  break;
	case 's':
	  opts.syslog = 1;
	  break;
	case 'f':
	  opts.fork = 1;
	  break;
	case 't':
          if (opts.nr_traphosts < 10) {
	    /* strlen doesn't count terminating \0. Add one to fix that. */
	    opts.traphosts[opts.nr_traphosts] = (char *)malloc(strlen(optarg)+1);
	    strncpy(opts.traphosts[opts.nr_traphosts], optarg, strlen(optarg)+1);
	    opts.nr_traphosts++;
	  }
	  else {
            fprintf(stderr, "ERROR: Not more than 10 trapdestinations can be specified");
          }
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
  printf("Checking for controllers.. \n");
  if (! discover_controllers(opts)) {
    printf("  None Found!\n\n");
    fprintf(stderr, "You don't seem to have any controllers\n");
    fprintf(stderr, "therefore it's rather senseless for me to run.\n\n");
    exit(1);
  }
  else {
    printf("Done\n");
  }

  printf ("Monitoring list\n");
  for (i=0; i<ctrls_found_num; i++) {
    printf (" [%2d] Controller type '%s' at %s\n",i, ctrls_found[i].ctrl_devicename,ctrls_found[i].devicefile);
  }

  /* get ip of current machine for traps */
  if (gethostname(buffer, HOST_NAME_MAX) == 0) {
    myhost = gethostbyname(buffer);
    myip = ((unsigned char) myhost->h_addr_list[0][3] << 24) +
      ((unsigned char) myhost->h_addr_list[0][2] << 16) +
      ((unsigned char) myhost->h_addr_list[0][1] << 8) +
      ((unsigned char) myhost->h_addr_list[0][0]);
  }
  else {
    perror("gethostname");
  }

  /* test for trap destinations */
  if (opts.debug) {
    for (i=0; i<opts.nr_traphosts; i++) {
      printf ("DEBUG: trap dest: %s\n", opts.traphosts[i]);
    }
  }
  
  /* set signal handler for KILL,HUP,TERM */
  memset(&myhandler, 0, sizeof (myhandler));
  myhandler.sa_handler = signal_handler;

  sigaction (SIGHUP, &myhandler, NULL);
  sigaction (SIGTERM, &myhandler, NULL);
  
  if (! opts.fork) {
    int fd, fdnum;

    result = fork();
    if (result < 0) {
      perror("fork");
      exit(1);
    }
    else if (result) {
      printf ("Pid is %d\n", result);
      pidfile = fopen ("/var/run/cpqarrayd.pid","w");
      fprintf (pidfile, "%d\n", result);
      fclose (pidfile);
      exit(0);
    }
    /* ADDITIONAL CODE FOR DAEMON */
    /* become process group leader */
    if (setsid() == -1) {
         fprintf(stderr,"bad setsid - %s\n",strerror(errno));
         exit(1);
       }
       /* if we are forked, we don't want to print out to stdout or stderr */
       fd=open("/dev/null", O_RDWR);
       dup2(fd, STDIN_FILENO);
       dup2(fd, STDOUT_FILENO);
       dup2(fd, STDERR_FILENO);
       close(fd);
       /* Close all unnecessary file descriptors */
       fdnum = getdtablesize();
       for ( fd = (STDERR_FILENO + 1); fd < fdnum; fd++ )
         close(fd);
       /* END OF ADDITIONAL CODE */
  }

  /* sprintf (buffer, "cpqarrayd[%d]\0", getpid); */
  openlog ("cpqarrayd", LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "Logging Enabled...");
  
  while (keeprunning) {
    status_check(opts);
    cciss_status_check(opts);
    if (keeprunning) { sleep(30); }
  }

  if ((access("/var/run/cpqarrayd.pid", R_OK | F_OK)) == 0)
    {
      unlink("/var/run/cpqarrayd.pid");
    }
  syslog (LOG_INFO, "Application terminated by signal.");
  closelog();

  return 0;   /* should return something */
}


    



	  
	  

		  
	      
	      
    



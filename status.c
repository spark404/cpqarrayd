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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ida_ioctl.h>
#include <ida_cmd.h>
#include <cpqarray.h>

#include "cpqarrayd.h"
#include "sendtrap.h"

int status_check (struct opts opts) 
{
  
  int devicefd;
  int ctrl_cntr;
  int logd_cntr;
  ida_ioctl_t io, io2;
  int status, nr_blks, blks_tr;
  float pvalue;
  char *statusmsg;
  
    
  for ( ctrl_cntr=0;
        ctrl_cntr <  ctrls_found_num;
        ctrl_cntr++) {
    
    devicefd = open (controllers[ctrl_cntr], O_RDONLY);
    
    for ( logd_cntr=0;
          logd_cntr < ctrls_found[ctrl_cntr].num_logd_found;
          logd_cntr++) {
      
        memset (&io, 0, sizeof (io));

        io.cmd = SENSE_LOG_DRV_STAT;
        io.unit = logd_cntr  | UNITVALID;
        
        if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
          {
            perror ("SENSE_LOG_DRV_STAT ioctl");
            return 0;
          }

        status=io.c.sense_log_drv_stat.status;
        
        if ((status == 3) || (status == 5) || (status == 7)) {
          /* is a progress indicator required?
           */
          memset (&io2, 0, sizeof (io));
          
          io2.cmd = ID_LOG_DRV;
          io2.unit = logd_cntr  | UNITVALID;
          
          if (ioctl (devicefd, IDAPASSTHRU, &io2) < 0)
            {
              perror ("ID_LOG_DRV ioctl");
              /* return 0;   no return this isn't fatal for now */
            }
          else 
            {
              nr_blks = io2.c.id_log_drv.nr_blks;
              blks_tr = io.c.sense_log_drv_stat.blks_to_recover;
                  
              pvalue = ((float)(nr_blks - blks_tr)/(float)nr_blks) * 100;

            }
        }
        else {
          pvalue = 0.0;
        }

        if (opts.debug) {
	  fprintf(stdout, "DEBUG: Status of controller %d unit %d is %d\n", 
		  ctrl_cntr, logd_cntr, status);
          fprintf(stdout, "DEBUG: ");
	  fprintf(stdout, statusstr[status], 
		  ctrl_cntr, logd_cntr, pvalue);
	  fprintf(stdout, "\n");
	}
	
	if (status != ctrls_found[ctrl_cntr].log_disk[logd_cntr].status) {
	  /* status changed, time to send a trap */
	  if (opts.debug) {
	    printf ("DEBUG: status changed from %d to %d, pvalue = %f\n",
		    ctrls_found[ctrl_cntr].log_disk[logd_cntr].status, status,
		    pvalue);
	  }
	  statusmsg = (char *)malloc(1024);
	  sprintf(statusmsg, statusstr[status], ctrl_cntr, logd_cntr, pvalue);
	  if (opts.debug) {
	    printf("DEBUG: sending trap to starbreeze.knoware.nl");
	  }
	  sendtrap(opts, "starbreeze.knoware.nl", "beheer", status, statusmsg);
	  sendtrap(opts, "clifford.knoware.nl", "beheer", status, statusmsg);
	  if (opts.debug) {
	    printf("DEBUG: trap sent to starbreeze.knoware.nl");
	  }
	}
	else if ((status == 5) && 
		 ((pvalue - ctrls_found[ctrl_cntr].log_disk[logd_cntr].pvalue)
		  >= 25.0 )) {
	  /* pvalue changed by more than 25%, time to send a trap */
	  if (opts.debug) {
	    printf ("DEBUG: pvalue changed from %f to %f\n",
		    ctrls_found[ctrl_cntr].log_disk[logd_cntr].pvalue,
		    pvalue);
	  }
	  statusmsg = (char *)malloc(1024);
	  sprintf(statusmsg, statusstr[status], ctrl_cntr, logd_cntr, pvalue);
	  if (opts.debug) {
	    printf("DEBUG: sending trap to starbreeze.knoware.nl");
	  }
	  sendtrap(opts, "starbreeze.knoware.nl", "beheer", status, statusmsg);
	  /* sendtrap(opts, "clifford.knoware.nl", "beheer", status, statusmsg);
	   */
	  if (opts.debug) {
	    printf("DEBUG: trap sent to starbreeze.knoware.nl");
	  }
	}
        
	ctrls_found[ctrl_cntr].log_disk[logd_cntr].status = status;
	ctrls_found[ctrl_cntr].log_disk[logd_cntr].pvalue = pvalue;
    }
    close (devicefd);
  }

  return 1;
 
}






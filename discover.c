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

int discover_controllers (struct opts);
int interrogate_controller (struct opts, int);
int interrogate_logical(struct opts, int, int);
void boardid2str (unsigned long , char *);

int
discover_controllers (struct opts opts)
{
  int cntr;
  int foundone = 0;

  for (cntr = 0; cntr < 8; cntr++)
    {
      /* does this device exist ? */
      if ((access (controllers[cntr], R_OK | F_OK)) == 0)
	{
	  /* it does :) */
	  if (interrogate_controller (opts, cntr))
	    {
	      foundone = 1;
	      if (opts.debug) 
		fprintf (stderr, "DEBUG: %s is a existing controller\n",
			 controllers[cntr]);
	    }
	}
      else if (opts.debug)
	{
	  fprintf (stderr, "DEBUG: Device %s could not be opened\n", controllers[cntr]);
	  perror ("DEBUG: reason");
	}
    }
   return foundone;
}

int
interrogate_controller (struct opts opts, int contrnum)
{
  int devicefd;
  ida_ioctl_t io;
  char buffer[30];
  int foundone = 0;
  int cntr;
 

  devicefd = open (controllers[contrnum], O_RDONLY);
  /* no checks, did that before */

  /* clear io */
  memset (&io, 0, sizeof (io));

  io.cmd = ID_CTLR;

  if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
    {
      if (opts.debug) perror ("DEBUG: ioctl");
      return 0;
    }

  boardid2str (io.c.id_ctlr.board_id, buffer);

  strncpy (ctrls_found[ctrls_found_num].ctrl_devicename, 
	   buffer, 20);

  ctrls_found[ctrls_found_num].num_logd_found = 0;

  for (cntr = 0; cntr < io.c.id_ctlr.nr_drvs; cntr++)
    {
      if (interrogate_logical (opts, devicefd, cntr))
	{
	  foundone = 1;
	}
    }

  if (opts.verbose) printf("  Found a %s (%d Logical drives)\n", buffer,
			   ctrls_found[ctrls_found_num].num_logd_found);

  ctrls_found_num++;

  if (!foundone)
    {
      /* No logical Drives !  */
    }

   close (devicefd);
  return 1;
}

int
interrogate_logical (struct opts opts, int devicefd, int unit_nr)
{
  ida_ioctl_t io;
  ida_ioctl_t io2;
  int nr_blks, blks_tr;

  if (opts.debug) printf ("DEBUG: interrogating unit %d\n", unit_nr);

  memset (&io, 0, sizeof (io));

  io.cmd = ID_LOG_DRV;
  io.unit = unit_nr | UNITVALID;

  if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
    {
      perror ("FATAL: ID_LOG_DRV ioctl");
      return 0;
    }

  memset (&io2, 0, sizeof (io2));

  io2.cmd = SENSE_LOG_DRV_STAT;
  io2.unit = unit_nr | UNITVALID;

  if (ioctl (devicefd, IDAPASSTHRU, &io2) < 0)
    {
      perror ("FATAL: SENSE_LOG_DRV_STAT ioctl");
      return 0;
    }
  
  ctrls_found[ctrls_found_num].num_logd_found++;
  /*  ctrls_found[ctrls_found_num].log_disk[unit_nr].status =
   * io2.c.sense_log_drv_stat.status;

   * nr_blks = io2.c.id_log_drv.nr_blks;
   * blks_tr = io.c.sense_log_drv_stat.blks_to_recover;
   * ctrls_found[ctrls_found_num].log_disk[unit_nr].pvalue =
   *  ((float)(nr_blks - blks_tr)/(float)nr_blks) * 100;
   */
  ctrls_found[ctrls_found_num].log_disk[unit_nr].status = 0;
  ctrls_found[ctrls_found_num].log_disk[unit_nr].pvalue = 0;

  return 1;
}

void
boardid2str (unsigned long board_id, char *name)
{

  switch (board_id)
    {

    case 0x0E114030:		/* SMART-2/E */
      strcpy (name, "Compaq SMART-2/E");
      break;
    case 0x40300E11:		/* SMART-2/P or SMART-2DH */
      strcpy (name, "Compaq SMART-2/P (2DH)");
      break;
    case 0x40310E11:		/* SMART-2SL */
      strcpy (name, "Compaq SMART-2SL");
      break;
    case 0x40320E11:		/* SMART-3200 */
      strcpy (name, "Compaq SMART-3200");
      break;
    case 0x40330E11:		/* SMART-3100ES */
      strcpy (name, "Compaq SMART-3100ES");
      break;
    case 0x40340E11:		/* SMART-221 */
      strcpy (name, "Compaq SMART-221");
      break;
    default:
      /*
       * Well, its a SMART-2 or better, don't know which
       * kind.
       */
      strcpy (name, "Unknown Controller Type");
    }
}







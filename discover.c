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

#include "cpqarrayd.h"

char *controllers[] =
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
	  if (interrogate_controller (cntr))
	    {
	      foundone = 1;
	      fprintf (stderr, "DEBUG: %s is a existing controller\n",
		       controllers[cntr]);
	    }
	}
      else
	{
	  fprintf (stderr, "Device %s could not be opened\n", controllers[cntr]);
	  perror ("reason");
	}
    }
   return foundone;
}

int
interrogate_controller (int contrnum)
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
      perror ("ioctl");
      return 0;
    }

  boardid2str (io.c.id_ctlr.board_id, buffer);

  strncpy (ctrls_found[ctrls_found_num].ctrl_devicename, 
	   buffer, 20);
  ctrls_found[ctrls_found_num].num_logd_found = 0;

  for (cntr = 0; cntr < io.c.id_ctlr.nr_drvs; cntr++)
    {
      if (interrogate_logical (ctrl_subtree, devicefd, cntr))
	{
	  foundone = 1;
	}
    }

  ctrls_found_num++;

  if (!foundone)
    {
      /* No logical Drives !  */
    }

   close (devicefd);
  return 1;
}

int
interrogate_logical (GtkWidget * tree, int devicefd, int unit_nr)
{
  ida_ioctl_t io;
  ida_ioctl_t io2;

  char buffer[16];
  int cntr, bus;

  printf ("DEBUG: interrogating unit %d\n", unit_nr);

  memset (&io, 0, sizeof (io));

  io.cmd = ID_LOG_DRV;
  io.unit = unit_nr | UNITVALID;

  if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
    {
      perror ("ID_LOG_DRV ioctl");
      return 0;
    }

  memset (&io2, 0, sizeof (io2));

  io2.cmd = SENSE_CONFIG;
  io2.unit = unit_nr | UNITVALID;

  if (ioctl (devicefd, IDAPASSTHRU, &io2) < 0)
    {
      perror ("SENSE_CONFIG ioctl");
      return 0;
    }

  sprintf (buffer, "Logical Drive %d", unit_nr);

  ctrls_found[ctrls_found_num].num_logd_found++;

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


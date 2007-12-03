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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include <linux/cciss_ioctl.h>
#include "cciss_structs.h"


const char *logicaldrivestatusstr[] = {
        "Logical drive is ok",
        "Logical drive is failing",
        "Logical drive is not configured",
        "Logical drive is using interim recovery mode",
        "Logical drive is ready for recovery operation",
        "Logical drive is is currently recovering",
        "Wrong physical drive was replaced",
        "A physical drive is not properly connected",
        "Hardware is overheating",
        "Hardware has overheated",
        "Logical drive is currently expanding",
        "Logical drive is not yet available",
        "Logical drive is queued for expansion",
};

const char *sparestatusstr[] = {
	"online",
};

const char *cciss_get_logical_drive_statusstr(int statuscode) {
  return logicaldrivestatusstr[statuscode];
}

const char *cciss_get_spare_statusstr(int statuscode) {
  return sparestatusstr[statuscode];
}

int
cciss_get_logical_luns (int device_fd, cciss_report_logicallun_struct * logluns)
{
	int result, outfile;
	IOCTL_Command_struct iocommand;
	unsigned char buffer[128];

	iocommand.LUN_info.LunAddrBytes[0] = 0;
	iocommand.LUN_info.LunAddrBytes[1] = 0;
	iocommand.LUN_info.LunAddrBytes[2] = 0;
	iocommand.LUN_info.LunAddrBytes[3] = 0;
	iocommand.LUN_info.LunAddrBytes[4] = 0;
	iocommand.LUN_info.LunAddrBytes[5] = 0;
	iocommand.LUN_info.LunAddrBytes[6] = 0;
	iocommand.LUN_info.LunAddrBytes[7] = 0;

	iocommand.Request.Type.Type = TYPE_CMD;
	iocommand.Request.Type.Attribute = ATTR_SIMPLE;
	iocommand.Request.Type.Direction = XFER_READ;

	iocommand.Request.Timeout = 0;	/* don't time out */

	iocommand.Request.CDBLen = 12;
	iocommand.Request.CDB[0] = 0xC2; /* Report logical LUNs */
	iocommand.Request.CDB[1] = 0x0;	 /* reserved, leave 0 */
	iocommand.Request.CDB[2] = 0x0;	 /* reserved, leave 0 */
	iocommand.Request.CDB[3] = 0x0;	 /* reserved, leave 0 */
	iocommand.Request.CDB[4] = 0x0;  /* reserved, leave 0 */
	iocommand.Request.CDB[5] = 0x0;  /* reserved, leave 0 */
	iocommand.Request.CDB[6] = 0x0;  /* byte 6-9 alloc length = 128 (0x80)*/
	iocommand.Request.CDB[7] = 0x0;
	iocommand.Request.CDB[8] = 0x0;
	iocommand.Request.CDB[9] = 0x80;
	iocommand.Request.CDB[10] = 0x0; /* reserved, leave 0 */
	iocommand.Request.CDB[11] = 0x0; /* control ? */

	memset (buffer, 0x0, 128);
	iocommand.buf_size = 128;
	iocommand.buf = buffer;

	result = ioctl (device_fd, CCISS_PASSTHRU, &iocommand);
	if (result < 0)
	{
		return -1;
	}

	/* Data underrun is OK since we do not proccess data */
	if (iocommand.error_info.CommandStatus != 0 && iocommand.error_info.CommandStatus != 2)
	{
		printf ("FATAL: 'Report Logical LUNs' failed with comnmand Status %d\n", iocommand.error_info.CommandStatus);
		return -2;
	}
	
 	memcpy (logluns, buffer, 128);
	return 0;
}

int
cciss_get_event (int device_fd, int reset_pointer, cciss_event_type * event)
{
	int result, outfile;
	IOCTL_Command_struct iocommand;
	unsigned char buffer[512];

	iocommand.LUN_info.LunAddrBytes[0] = 0;
	iocommand.LUN_info.LunAddrBytes[1] = 0;
	iocommand.LUN_info.LunAddrBytes[2] = 0;
	iocommand.LUN_info.LunAddrBytes[3] = 0;
	iocommand.LUN_info.LunAddrBytes[4] = 0;
	iocommand.LUN_info.LunAddrBytes[5] = 0;
	iocommand.LUN_info.LunAddrBytes[6] = 0;
	iocommand.LUN_info.LunAddrBytes[7] = 0;

	iocommand.Request.Type.Type = TYPE_CMD;
	iocommand.Request.Type.Attribute = ATTR_SIMPLE;
	iocommand.Request.Type.Direction = XFER_READ;

	iocommand.Request.Timeout = 0;	/* don't time out */

	iocommand.Request.CDBLen = 13;
	iocommand.Request.CDB[0] = 0xC0;	/* CISS Read */
	iocommand.Request.CDB[1] = 0xD0;	/* Notify on Event */
	iocommand.Request.CDB[2] = 0x0;	/* reserved, leave 0 */
	iocommand.Request.CDB[3] = 0x0;	/* reserved, leave 0 */
	iocommand.Request.CDB[4] = 0x0;
	iocommand.Request.CDB[5] = 0x0;
	iocommand.Request.CDB[6] = 0x0;
	iocommand.Request.CDB[7] = (reset_pointer) ? 0x7 : 0x3;	/* 7 = start at oldest, 3 is get current */
	iocommand.Request.CDB[8] = 0x0;
	iocommand.Request.CDB[9] = 0x0;
	iocommand.Request.CDB[10] = 0x2;
	iocommand.Request.CDB[11] = 0x0;
	iocommand.Request.CDB[12] = 0x0;

	memset (buffer, 0x0, 512);
	iocommand.buf_size = 512;
	iocommand.buf = buffer;

	result = ioctl (device_fd, CCISS_PASSTHRU, &iocommand);
	if (result < 0)
	{
		perror (" * ioctl failed");
		return -1;
	}

	if (iocommand.error_info.CommandStatus == 1) {
		printf (" * Command succeeded with dataoverrun (code %d)\n", iocommand.error_info.CommandStatus);
	}
	else if (iocommand.error_info.CommandStatus == 2) {
		printf (" * Command succeeded with dataunderrun (code %d)\n", iocommand.error_info.CommandStatus);
	}
	else if (iocommand.error_info.CommandStatus != 0)
	{
		printf (" * Command failed with Comnmand Status %d\n", iocommand.error_info.CommandStatus);
		return -1;
	}

	memcpy (event, buffer, 512);
	return 0;
}


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

extern int cciss_get_logical_luns (int, cciss_report_logicallun_struct *);
extern int cciss_get_event (int, int, cciss_event_type *);
extern const char *cciss_get_logical_drive_statusstr(int );
extern const char *cciss_get_spare_statusstr(int );

extern const char *logicaldrivestatusstr[];

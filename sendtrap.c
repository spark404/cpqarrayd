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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>

#include <ucd-snmp/ucd-snmp-config.h>
#include <ucd-snmp/asn1.h>
#include <ucd-snmp/mib.h>
#include <ucd-snmp/snmp.h>
#include <ucd-snmp/snmp_api.h>
#include <ucd-snmp/snmp_impl.h>
#include <ucd-snmp/snmp_client.h>

#include "cpqarrayd.h"

int sendtrap(struct opts opts, char *peer, char *community, 
	     int status, char *message)
{
  struct snmp_session session, *ss;
  struct snmp_pdu *pdu;    
  oid enterprise[] = {1,3,6,1,4,1,300};

  memset(&session, 0, sizeof(struct snmp_session));

  session.peername = peer;
  session.community = community;
  session.community_len = 6;
  session.version = SNMP_VERSION_1;
  session.retries = 5; 
  session.timeout = 500;
  session.remote_port = 162;
  session.authenticator = NULL;

  /*  snmp_synch_setup(&session);   Whats this for ? */

  ss = snmp_open(&session);   
  if (ss == NULL) {
    fprintf(stderr, "Couln't open snmp!\n");
    exit(1);
  }

  pdu = snmp_pdu_create(SNMP_MSG_TRAP);
  pdu->agent_addr.sin_addr.s_addr = 119816387;
  pdu->enterprise = enterprise;
  pdu->enterprise_length = sizeof(enterprise) / sizeof (oid);
  pdu->trap_type = 6;
  pdu->specific_type = 1; 
  pdu->time = 0;

  snmp_send(ss, pdu);
  // snmp_perror("snmp_send");
  snmp_close(ss);

  return (0);
}

	
	

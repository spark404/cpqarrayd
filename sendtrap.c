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

#ifdef HAVE_SNMPTRAP
  #include <net-snmp/net-snmp-config.h>
  #include <net-snmp/net-snmp-includes.h>
  #undef DEMO_USE_SNMP_VERSION_3
#endif


#include "cpqarrayd.h"

int sendtrap(struct opts opts, char *community, 
	     int status, char *message)
{
  int return_stat = 0;

#ifdef HAVE_SNMPTRAP
  struct snmp_session session, *ss;
  struct snmp_pdu *pdu;    
  char statusmsg[12];
  oid enterprise[] = {1,3,6,1,4,1,300};
  oid statusoid[] = {1,3,6,1,4,1,300,1};
  oid messageoid[] = {1,3,6,1,4,1,300,2};
  int counter;
  char *messagebuf;
  in_addr_t *pdu_in_addr_t;

  
  for (counter=0; counter < opts.nr_traphosts; counter++) {
    
    snmp_sess_init( &session );
    
    /* strlen() doesn't count the terminating \0, so we do +1 in malloc. */
    session.peername = (char *)malloc(strlen(opts.traphosts[counter])+1);
    strcpy (session.peername, opts.traphosts[counter]);
    session.community = (char *)malloc(strlen(community)+1);
    strcpy (session.community, community);
    session.community_len = strlen(session.community);
    session.version = SNMP_VERSION_1;
    session.retries = 5; 
    session.timeout = 500;
    session.remote_port = 162;
    session.authenticator = NULL;
    
    ss = snmp_open(&session);   
    if (!ss) {
      snmp_sess_perror( "Error: sendtrap/snmp_open", &session );
      return_stat++;
    }
    
    pdu = snmp_pdu_create(SNMP_MSG_TRAP);
    pdu_in_addr_t = (struct in_addr_t *)&pdu->agent_addr;
    pdu->enterprise = malloc(sizeof(enterprise));
    memcpy (pdu->enterprise, enterprise, sizeof(enterprise));
    pdu->enterprise_length = sizeof(enterprise) / sizeof (oid);
    pdu->trap_type = 6;
    pdu->specific_type = 1; 
    pdu->time = 0;
    pdu->contextEngineID = 0x0;
    *pdu_in_addr_t = get_myaddr();

    sprintf(statusmsg, "%d", status);

    snmp_add_var (pdu, statusoid, sizeof(statusoid) / sizeof (oid), 'i', 
		  statusmsg);
    messagebuf = (char *)malloc(strlen(message)+1);
    strcpy(messagebuf,message);
    snmp_add_var (pdu, messageoid, sizeof(messageoid) / sizeof (oid), 's', 
		  message);
    
    if (!snmp_send(ss, pdu)) {
      snmp_sess_perror("Error: sendtrap/snmp_send", ss );
      return_stat++;
    }
    snmp_close(ss);
  }
#endif
  return return_stat;
}

	
	

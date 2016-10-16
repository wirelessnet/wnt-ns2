/*
This software is designed by Jorge Hortelano Otero.
softwaremagico@gmail.com
Copyright (C) 2008 Jorge Hortelano Otero.
C/Botanico 12, 1. Valencia CP:46008 (Spain).
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
Created on November of 2009.
 */

/***********************************

                    MACROS

 ***********************************/
 
#ifndef CONFIG_H
#define CONFIG_H



#define WATCHDOG_VERSION "2.00"
#define NEIGHBOUR_TIMEOUT 10		/* Time to delete a neighbour when the watchdog does not listen any new packet */
#define OBSERVATION_TIME 2		/* Time to upgrade Beta function */
#define DEBUGGER 0				/* Level of verbosity */
#define FINISH 0				/* Finish program when detecting a blackhole */
#define STORE_PACKET_TIME 20		/* Time to store a packet to forward. When this time is elapsed, the stored packet is erased and is considered a packet lost */
#define STORE_ROUTE_TIME 30		/* Time to store a the routes that pass through a node. When this time is elapsed without a new paquet, the route is erased */
#define PERCENTAGE_LOSS 20
#define SEARCH_PACKET_TIME 10         /* Only compare the packets on this last time to see if the packet is repeated.  */
#define MAX_SLEEP	1
#define MIN_SLEEP	0
#define PCK_BURST	200
#define GW_IP	"127.0.0.1"
#define PORTW	1999
#define MAX_PROTOCOL_NAME 16
#define HEARTBEAT_MALICIOUS_NODE 60	/* For a detected malicious node, min time to repeat an alert */
#define IMMEDIATE_REATION_DURATION 0	/* Duration of the immediate reaction */
#define MIN_SIGNIFICANT_PACKETS 30
#define WEIGHT_OF_OLD_PACKETS   10
#define SHOW_STATISTICS 1               	/* Show statistics when the watchdog is finished */
#define STORING_TIME_BETA_PCK 5
#define GAMMA 0.85
#define BETA_DISCOUNT 0.9
#define STANDARD_WATCHDOG 0
#define BAYESIAN_WATCHDOG 1
#define NUMBER_PROTOCOLS 5
#define MAX_PROTOCOL_NAME 10


#endif

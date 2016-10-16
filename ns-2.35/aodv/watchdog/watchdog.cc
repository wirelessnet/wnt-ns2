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

#include "watchdog.h"

bool
WATCHDOG::file_exists(const char *filename) {
	if (FILE *file = fopen(filename, "r")) { //I'm sure, you meant for READING =)
		fclose(file);
		return true;
	}
	return false;
}


/**
 * Determines if a packet is interesting for a net.
 */
int
WATCHDOG::isInterestingPacket(int32_t source_ip, int source_mac, int32_t destination_ip, int destination_mac) {
	/* No broadcast */
	if (destination_ip != -1 && source_mac != -1 && destination_mac != -1) {
		return 1;
	}

	return 0;
}


/**
 * A new packet has been sended, i must store it until confirmation of forward.
 */
void
WATCHDOG::waitingForwardPacket(int32_t source_ip, int32_t destination_ip, int sourcePort, int destinationPort, char *body, int data_size, neighbour *neigh, int protocol, double pktime) {
	if (debug > 3) printf("waitingForwardPacket\n");
	if (neigh != NULL) {
		neigs->AddNeighbourPacketToForward(neigh, body, destination_ip, source_ip, data_size, storing_packet_timeout, protocol, sourcePort, destinationPort, pktime);
		neigs->AddFlow(source_ip, destination_ip, neigh, pktime);
	}
}


/**
 * A packet has been forwarded, delete it from the waiting forward list.
 */
void
WATCHDOG::packetForwarded(int32_t destination_ip, int32_t source_ip, char *body, int data_size, neighbour *neigh, int protocol, double pktime) {
	if (debug > 3) printf("packetForwarded\n");
	if (neigh != NULL) {
		neigs->AddNeighbourPacketForwarded(neigh, body, destination_ip, source_ip, data_size, storing_packet_timeout, protocol, pktime);
	}
}


/**
 * A packet from a neighbour has been detected.
 */
void
WATCHDOG::neighboursPacketDetected(int32_t source_ip, int32_t destination_ip, int source_mac, int destination_mac,  int sourcePort, int destinationPort, char *tmp_data, packet_t packet_type, double pktime) {
	if (debug > 3) printf("NeighbourPacket\n");
	neighbour *neigh_src, *neigh_dst;
	int data_size;

	data_size = strlen(tmp_data);
	protocol = packet_type;

	neigh_src = neigs->ExistNeighbourMac(source_mac);
	neigh_dst = neigs->ExistNeighbourMac(destination_mac);

	if (isInterestingPacket(source_ip, source_mac, destination_ip, destination_mac)) {
		/* If the destination IP is different of the destination neighbour, means that must be forwarded */
		if (neigh_dst != NULL && (neigh_dst->ip != 666) && (destination_ip != neigh_dst->ip)) {
			waitingForwardPacket(source_ip, destination_ip, sourcePort, destinationPort, tmp_data, data_size, neigh_dst, protocol, pktime);
		}
		/* If the IP source is different of the sender, means that is a forwarded packet */
		if ((neigh_src != NULL) && (neigh_src->ip != 666)) {
			if (debug > 3)  printf("Sender: %d Hop: %d Destination: %d\n", source_mac, neigh_src->mac, destination_ip);
			if (source_ip != neigh_src->ip) {
				packetForwarded(destination_ip, source_ip, tmp_data, data_size, neigh_src, protocol, pktime);
			}
		}
	}
}


/**
 * I read my own packet.
 */
void
WATCHDOG::ownPacketDetected(int32_t source_ip, int32_t destination_ip, int sourcePort, int destinationPort, int mac_dst, char *tmp_data, packet_t packet_type, double pktime) {
	if (debug > 3) printf("Ownpacket\n");
	int data_size;
	neighbour *neigh_dst;

	data_size = strlen(tmp_data);
	protocol = packet_type;

	neigh_dst = neigs->ExistNeighbourMac(mac_dst);

	if (mac_dst != 666) {
		/* If the destination IP is different of the destination neighbour, means that must be forwarded */
		//if (neigh_dst != NULL){
		if (neigh_dst != NULL && (neigh_dst->ip != 666) && (destination_ip != neigh_dst->ip)) {
			waitingForwardPacket(source_ip, destination_ip, sourcePort, destinationPort, tmp_data, data_size, neigh_dst, protocol, pktime);
		}
	}
}


/**
 * Read one packet of the network device.
 */
void
WATCHDOG::readPacket(int32_t source_ip, int32_t destination_ip, int source_mac, int destination_mac, int sourcePort, int destinationPort, packet_t packet_type, char *tmp_data, double pktime) {
	char text[250];
	char path[250];
	FILE *f;

	if (debug > 3) printf("Reading packet\n");
	readed_packets++;

	if (packet_type == PT_ARP) { /* Is a ARP packet*/
		arp_packets++;
		if (source_ip == ownIP) { /* is my own ARP packet*/
			//ownMAC = source_mac;
		} else { /* Is a neighbour ARP packet */
			neigs->AssignIpToNeighbour(source_mac, source_ip, isInterestingPacket(source_ip, source_mac, destination_ip, destination_mac), pktime, ownMAC);
			/* Show debug data */
			if ((debug > 1) && isInterestingPacket(source_ip, source_mac, destination_ip, destination_mac)) {
				//PrintArpPacketInformation(n);
				//ShowAllNeighbours(&neigs);
			}
		}
	} else if (packet_type == PT_AODV) { /* Is a AODV packet */
		if (destination_ip == -1 && destination_mac == -1 && sourcePort == 255 && destinationPort == 255 && packet_type == 38) { /* Broadcast AODV packets, are sended by the own node.*/
			neigs->AssignIpToNeighbour(source_mac, source_ip, 1, pktime, ownMAC);
		}
	} else { /* Is a IP packet */
		ip_packets++;
		/* is my packet */
		if (source_mac == ownMAC) {
			ownPacketDetected(source_ip, destination_ip, sourcePort, destinationPort, destination_mac, tmp_data, packet_type, pktime);
			interesting_readed_packets++;
		} else {
			/* I am the last node of the traffic flow*/
			if (destination_ip == ownIP) {
				if (lastnode == 0) {
					lastnode = 1;
					sprintf(text, "Flow received by node %d at %f!\n", ownMAC, pktime);
					sprintf(path, "flows.txt", "", sufix, sufix);
					if (file_exists(path)) {
						f = fopen(path, "a");
						fprintf(f, "%s", text);
						if (source_mac == source_ip) {
							sprintf(text, "Direct delivery from %d at %f!\n", source_mac, pktime);
							fprintf(f, "%s", text);
						}
						fclose(f);
					}
				}
			}
			if (isInterestingPacket(source_ip, source_mac, destination_ip, destination_mac)) {
				interesting_readed_packets++;
				neigs->AssignIpToNeighbour(source_mac, source_mac, isInterestingPacket(source_ip, source_mac, destination_ip, destination_mac), pktime, ownMAC);   //This is because ns2 does not work well and the watchdog may be can not listen the AODV or ARP packets to obtain the IP of a node. But, we it is equal to the MAC!!!!
				neighboursPacketDetected(source_ip, destination_ip, source_mac, destination_mac, sourcePort, destinationPort, tmp_data, packet_type, pktime);
			} else if (source_ip != -1 && source_mac != -1 && (destination_mac == -1)) {
				/* Broadcast message*/
				neigs->AssignIpToNeighbour(source_mac, source_ip, 1, pktime, ownMAC);
			}
		}
	}
}

void
WATCHDOG::updateMAC(int mac) {
	ownMAC = mac;
}

void WATCHDOG::changeSufix(int tmp_sufix) {
	sufix = tmp_sufix;
}

/**
* Obtain gamma value
*/
void
WATCHDOG::UpdateVariables(int sufix) {
	char path[250];
	char str[100];
	char line[100];
	FILE *file=NULL;

	sprintf(path, "variables.txt", "", sufix);

	gamma = GAMMA;
	fading = BETA_DISCOUNT;
	neighbour_timeout = NEIGHBOUR_TIMEOUT; //timeout for neighbours stored.
	observation_time = OBSERVATION_TIME;

	if ((file = fopen(path, "r"))!=NULL) {
		
		if(fgets(str, 100, file)!=NULL){ /* Activate the standard, the bayesian watchdog or both of them */
			if(strstr(str,"standard")!=NULL){
				standard = 1;
				bayesian = 0;
			}
			
			if(strstr(str,"bayesian")!=NULL){
				bayesian = 1;
			}
		}

		if (fgets(str, 100, file)!=NULL) {
			gamma = atof(str);
		}

		if (fgets(str, 100, file)!=NULL) {
			fading = atof(str);
		}
		
		if (fgets(str, 100, file)!=NULL) {
			neighbour_timeout = atof(str);
			if(observation_time >= neighbour_timeout){
				observation_time = neighbour_timeout / 2;
			}
		}
		
		if (fgets(str, 100, file)!=NULL) {
			observation_time = atof(str);
			if(observation_time >= neighbour_timeout){
				neighbour_timeout = observation_time * 2;
			}
		}

		fclose(file);
	}

}


/*
New Packet detected!
*/
void
WATCHDOG::newPacket(int32_t source_ip, int32_t destination_ip, int source_mac, int destination_mac, int sourcePort, int destinationPort,  packet_t packet_type, char *tmp_data, double tmp_time) {
	if (debug > 3) printf("Node IP: %d MAC: %d says:\n", ownIP, ownMAC);
	readPacket(source_ip, destination_ip, source_mac, destination_mac, sourcePort, destinationPort, packet_type, tmp_data, tmp_time);
	if (debug > 3) printf("Search for black-hole\n"); 
	if (standard) neigs->SearchForBlackHole(percentile_of_loss, ownIP, heartbeat_malicious, immediate_reaction_duration,  tmp_time, sufix);

	/* Update beta function */
	if (debug > 3) printf("Update beta\n");
	if (bayesian) neigs->UpdateBetaOfNeighbourhood(fading, STORING_TIME_BETA_PCK,  tmp_time, observation_time);
	if (debug > 3) printf("Search for black-hole with beta\n");
	if (bayesian) neigs->SearchForBlackHoleUsingBeta((float)percentile_of_loss, ownIP, heartbeat_malicious, immediate_reaction_duration,  tmp_time, sufix, STORING_TIME_BETA_PCK);
	if (debug > 3) printf("DeleteOldNeighbours\n");
	neigs->DeleteOldNeighbours(neighbour_timeout, debug, tmp_time, ownIP);
	if (debug > 3) printf("RemoveOldPackets\n");
	neigs->RemoveOldPackets(storing_packet_timeout, tmp_time);
	if (debug > 3) printf("RemoveOldFlows\n");
	neigs->RemoveOldFlows(storing_route_timeout, tmp_time);

	/* Apply devaluation technique */
	if (devalue && standard) { 
		neigs->CompressPacketsAllNeighbours(reduction);
	}
	if (debug > 3) neigs->ShowAllNeighbours(PERCENTAGE_LOSS, tmp_time);
	if (debug == 3) neigs->ShowAllInterestingNeighbours(PERCENTAGE_LOSS, ownIP, ownMAC, tmp_time);

}



/*
Constructor
*/
WATCHDOG::WATCHDOG(int32_t own_IP, int sufix) {
	port = PORTW; //Port of the GW.
	devalue = 0; //Devalue old packets.
	readed_packets = 0;
	ip_packets = 0;
	arp_packets = 0;
	interesting_readed_packets = 0;
	heartbeat_malicious = HEARTBEAT_MALICIOUS_NODE;
	immediate_reaction_duration = IMMEDIATE_REATION_DURATION;
	percentile_of_loss = PERCENTAGE_LOSS;
	storing_packet_timeout = STORE_PACKET_TIME;
	storing_route_timeout = STORE_ROUTE_TIME;
	reduction = WEIGHT_OF_OLD_PACKETS; //Weight of the olds packets when are devaluated.
	changeSufix(sufix);
	standard = STANDARD_WATCHDOG;
	bayesian = BAYESIAN_WATCHDOG;

	UpdateVariables(sufix);
	neigs = new NEIGHBOURS(gamma);
	ownIP = own_IP;
	ownMAC = 666;
	lastnode = 0;
	debug = DEBUGGER;

}



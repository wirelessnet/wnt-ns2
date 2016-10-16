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

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include "neighbours.h"
#include<stdlib.h>
#include<packet.h>
#include <signal.h>


#define DEBUGGER 3

class WATCHDOG {

public:
	WATCHDOG(int32_t own_IP, int sufix);
	void updateMAC(int mac);
	void newPacket(int32_t source_ip, int32_t destination_ip, int source_mac, int destination_mac, int sourcePort, int destinationPort,  packet_t packet_type, char *tmp_data, double tmp_time);
	void changeSufix(int tmp_sufix);
	//extern int **voting;
	float fading;
	float gamma;

protected:
	int devalue; //Devalue old packets.
	int port ; //Port of the GW.
	int heartbeat_malicious;
	int immediate_reaction_duration ;
	int percentile_of_loss ;

private:
	NEIGHBOURS *neigs;
	int32_t ownIP;
	int ownMAC;
	float readed_packets;
	float ip_packets;
	float arp_packets;
	float interesting_readed_packets;
	float neighbour_timeout; //timeout for neighbours stored.
	float observation_time; //timeout for neighbours stored.
	int storing_packet_timeout ;
	int storing_route_timeout ;
	int reduction; //Weight of the olds packets when are devaluated.
	int debug;
	int protocol;
	int lastnode;
	int sufix;
	int standard;
	int bayesian;

	void readPacket(int32_t source_ip, int32_t destination_ip, int source_mac, int destination_mac, int sourcePort, int destinationPort, packet_t packet_type, char *tmp_data, double pktime);
	void ownPacketDetected(int32_t source_ip, int32_t destination_ip, int sourcePort, int destinationPort, int mac_dst, char *tmp_data, packet_t packet_type, double pktime);
	void neighboursPacketDetected(int32_t source_ip, int32_t destination_ip, int source_mac, int destination_mac,  int sourcePort, int destinationPort, char *tmp_data, packet_t packet_type, double pktime);
	void packetForwarded(int32_t destination_ip, int32_t source_ip, char *body, int data_size, neighbour *neigh, int protocol, double pktime);
	void waitingForwardPacket(int32_t source_ip, int32_t destination_ip, int sourcePort, int destinationPort, char *body, int data_size, neighbour *neigh, int protocol, double pktime);
	int isInterestingPacket(int32_t source_ip, int source_mac, int32_t destination_ip, int destination_mac);
	void UpdateVariables(int sufix);
	bool file_exists(const char *filename);
};

#endif /* __WATCHDOG_H__ */

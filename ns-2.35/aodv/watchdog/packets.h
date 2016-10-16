#ifndef __PACKETS_H__
#define __PACKETS_H__


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
Created on June of 2008.
 */


#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<malloc.h>

#define min(A,B) ((A)<(B)? A : B)
#define NUMBER_PROTOCOLS 256
#define STARTING_STORED_PACKETS 0
#define MAX_FORWARDED_PACKETS 50000   //This value must be greater than packets received per second * seconds that the watchdog stores a packet!


typedef struct packet {
	double time;
	char *data;
	int32_t ip_src;
	int32_t ip_dst;
	int data_size;
	int protocol;
	int originPort;
	int destinationPort;
	struct packet *next;
	struct packet *previous;
	float devalue;
} packet;


typedef struct packetsListened {
	float received_packets[NUMBER_PROTOCOLS]; /* Packets received by each protocol. */
	float forwarded_packets[NUMBER_PROTOCOLS]; /* Packets forwarded by each protocol. */
	float stored_packets_protocol[NUMBER_PROTOCOLS];
	float stored_packets;
	packet * packets[NUMBER_PROTOCOLS];
	packet * last_packet[NUMBER_PROTOCOLS]; /* Pointer to the last packet inserted*/
} packetsListened;


class PACKETS {
public:
	PACKETS();
	~PACKETS();
	void CompressPackets(int reduction);
	void AddPacketForwarded(char *body, int32_t ip_dst, int ip_src, int data_size, int storing_time, int protocol,  double pktime);
	void AddPacketToForward(char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, int originPort, int destinationPort, double pktime) ;
	packet * ExistsPacket(char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, double pktime) ;
	float stored_packets_protocol(int protocol);
	float forwarded_packets(int protocol);
	float received_packets(int protocol);
	float stored_packets();
	void RemovePacket(packet *pk, int protocol);
	packet* get_packet(int index);
	void DeletePacketsListened();

protected:
private:
	packetsListened *pckts;
	void PrintPacketsSotored(packetsListened *pckts);
	packet* ShortExistsPacket(char *body, int32_t ip_dst, int ip_src, int data_size, int max_time, int protocol, double pktime);
	void DevalueOldStoredPackets( int protocol, int devalue);
	void DeletePacket(packet *p);
	void error(char *msg);
};


#endif /* __PACKETS_H__ */

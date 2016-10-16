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

#include"packets.h"

float max_packets_lost = 0; //Max number of packets stored
unsigned int starting_forwarded_packets = STARTING_STORED_PACKETS;
unsigned int max_forwarded_packets = MAX_FORWARDED_PACKETS;


/**
* Show the error
*/
void
PACKETS::error(char *msg) {
	perror(msg);
	exit(-1);
}


/**
 * Free al pointers of a packet.
 */
void
PACKETS::DeletePacket(packet *p) {
	free(p->data);
	free(p);
}


/**
 * Delete all packets.
 */
void
PACKETS::DeletePacketsListened() {
	int i;
	packet *p, *tmp_p;

	for (i = 0; i < NUMBER_PROTOCOLS; i++) {
		p = pckts->packets[i];
		while (p != NULL) {
			tmp_p = p;
			p = p->next;
			DeletePacket(tmp_p);
		}
	}
	free(pckts);
}


/**
 * Delete a stored packet.
 */
void
PACKETS::RemovePacket( packet *pk, int protocol) {
	packet *next, *prev;
	if (pk != NULL) {
		next = pk->next;
		prev = pk->previous;

		/* is the first packet */
		if (pk == pckts->packets[protocol]) {
			pckts->packets[protocol] = pk->next;
		}

		/* is the last packet */
		if (pk == pckts->last_packet[protocol]) {
			pckts->last_packet[protocol] = pk->previous;
		}

		if (next != NULL) next->previous = prev;
		if (prev != NULL) prev->next = next;

		pckts->stored_packets--;
		if(pk->devalue != 0){
			pckts->stored_packets_protocol[protocol] = pckts->stored_packets_protocol[protocol] - (1 / pk->devalue);
		}
		DeletePacket(pk);
		pk = NULL;

	} else {
		printf("This packet does not exists!\n");
	}
}


/**
 * Search for a stored packet.
 */
packet *
PACKETS::ExistsPacket( char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, double pktime) {
	if (pckts->packets == NULL) return NULL;
	packet *p, *tmp_p;

	p = pckts->packets[protocol];
	while (p != NULL) {
		/* Remove old packets */
		if (pktime - p->time > storing_time) {
			tmp_p = p;
			p = p->next;
			RemovePacket(tmp_p, protocol);
			continue;
		}

		if (!strncmp(p->data, body, data_size)) {
			if (p->ip_src == ip_src) {
				if (p->ip_dst == ip_dst) {
					return p;
				}
			}
		}
		p = p->next;
	}
	return NULL;
}


/**
 * Reduce the value of a packet. Useful to limitating the value of old packets when is lots of new traffic. Used to paliate the false negative problem.
 */
void
PACKETS::DevalueOldStoredPackets( int protocol, int devalue) {
	packet *p;

	p = pckts->packets[protocol];
	while (p != NULL) {
		p->devalue *= devalue;
		p = p->next;
	}
}


/**
 * Search for a recently stored packet (Only the last SHORT_TIME seconds are checked).
 */
packet *
PACKETS::ShortExistsPacket(char *body, int32_t ip_dst, int32_t ip_src, int data_size, int max_time, int protocol, double pktime) {
	if (pckts->packets == NULL) return NULL;
	packet *p;

	p = pckts->last_packet[protocol];
	while (p != NULL && (pktime - p->time < max_time)) {
		if (!strncmp(p->data, body, data_size)) {
			if (p->ip_src != ip_src) {
				if (p->ip_dst != ip_dst) {
					return p;
				}
			}
		}
		p = p->previous;
	}
	return NULL;
}


/**
 *  Indicates that a new packet that must to be forwarded has been listened.
 */
void
PACKETS::AddPacketToForward(char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, int originPort, int destinationPort, double pktime) {
	packet *pk;
	packet *exists;

	/* Sometimes can listen a packet two or more times, we must avoid insert the same packet two times */
	exists = ShortExistsPacket(body, ip_dst, ip_src, data_size, storing_time, protocol, pktime);
	if (exists == NULL) {
		if ((pk = (packet *) (malloc(sizeof (packet)))) < 0)
			error("Memory assignment of packet!\n");
		pk->time = pktime;
		pk->data = (char *)malloc(sizeof (char) * data_size);
		strncpy(pk->data, body, data_size);
		pk->ip_src = ip_src;
		pk->ip_dst = ip_dst;
		pk->data_size = data_size;
		pk->time = pktime;
		pk->originPort = originPort;
		pk->destinationPort = destinationPort;
		pk->protocol = protocol;
		pk->previous = NULL;
		pk->next = NULL;
		pk->devalue = 1;

		pckts->received_packets[protocol]++;
		pckts->stored_packets++;
		pckts->stored_packets_protocol[protocol]++;

		if (pckts->stored_packets > max_packets_lost) max_packets_lost = pckts->stored_packets;

		if (pckts->last_packet[protocol] == NULL) {
			pckts->packets[protocol] = pk;
			pckts->last_packet[protocol] = pk;
			pk->previous = NULL;
		} else {
			pckts->last_packet[protocol]->next = pk;
			pk->previous = pckts->last_packet[protocol];
			pckts->last_packet[protocol] = pk;
		}

	} else {
	}
}


/**
 * Indicates that a new packet forwarded has been listened.
 */
void
PACKETS::AddPacketForwarded( char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, double pktime) {
	packet * exists;
	exists = ExistsPacket(body, ip_dst, ip_src, data_size, storing_time, protocol, pktime);
	if (exists != NULL) {
		pckts->forwarded_packets[protocol]++;
		RemovePacket(exists, protocol);
	}
}


/**
 * Shows all packets stored.
 */
void
PACKETS::PrintPacketsSotored(packetsListened *pckts) {
	int i = 0;
	for (i = 0; i < pckts->stored_packets; i++) {
		printf("\tPacket: %d to %d: %s\n", pckts->packets[i]->ip_src, pckts->packets[i]->ip_dst, pckts->packets[i]->data);
	}
}


/**
 * Reduce the number of forwarded packets to avoid long time at false negatives.
 */
void
PACKETS::CompressPackets(int reduction) {
	int i;
	for (i = 0; i < NUMBER_PROTOCOLS; i++) {
		if ((pckts->forwarded_packets[i] > max_forwarded_packets) || (pckts->received_packets[i] > max_forwarded_packets)) {
			if(reduction != 0){
				pckts->received_packets[i] = pckts->received_packets[i] / reduction;
				pckts->forwarded_packets[i] = pckts->forwarded_packets[i] / reduction;
				pckts->stored_packets_protocol[i] = pckts->stored_packets_protocol[i] / reduction;
			}
			DevalueOldStoredPackets(i, reduction);
		}
	}
}

float
PACKETS::stored_packets_protocol(int protocol) {
	return pckts->stored_packets_protocol[protocol];
}

float
PACKETS::forwarded_packets(int protocol) {
	return pckts->forwarded_packets[protocol];
}

float
PACKETS::received_packets(int protocol) {
	return pckts->received_packets[protocol];
}

float
PACKETS::stored_packets() {
	return pckts->stored_packets;
}

packet *
PACKETS::get_packet(int index) {
	return pckts->packets[index];
}


/*
Constructor
*/
PACKETS::PACKETS() {
	int i;

	if ((pckts = (packetsListened *) (malloc(sizeof (packetsListened)))) < 0)
		error("Memory assignment of packet!\n");
	for (i = 0; i < NUMBER_PROTOCOLS; i++) {
		if ((i == 17) || (i == 6) || (i == 1)) { //For testing purpose, allow start protocols with already forwarded packets
			pckts->received_packets[i] = starting_forwarded_packets;
			pckts->forwarded_packets[i] = starting_forwarded_packets;
		} else {
			pckts->received_packets[i] = 0;
			pckts->forwarded_packets[i] = 0;
		}
		pckts->stored_packets_protocol[i] = 0;
		pckts->packets[i] = NULL;
		pckts->last_packet[i] = NULL;
	}
	pckts->stored_packets = 0;
}


PACKETS::~PACKETS() {
	DeletePacketsListened();
}



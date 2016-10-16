#ifndef __NEIGHBOUR_H__
#define __NEIGHBOUR_H__

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


#include<time.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include"packets.h"
#include"config.h"
#include"beta.h"


/* The neighbour has a list of flows to known how many routes affect. */
struct flow {
	double time;
	struct flow *next;
	struct flow *previous;
	int32_t destination;
	int32_t source;
};

struct neighbour {
	double time; /* The time where this neighbour has been sight the last time. */
	int mac;
	int32_t ip;
	PACKETS *pckts;
	double malicious_node;
	double malicious_node_bf;
	double immediate_reaction;
	struct neighbour *next;
	struct neighbour *previous;
	double starting_time;
	struct flow *flows;
	int number_flows;
	BETA *reputation;
	//BETA trust;
	//beta_packets_listened *beta_packets;
};

struct neighbours {
	int howmany;
	neighbour *group;
	neighbour *last;
};


class NEIGHBOURS {
public:
	NEIGHBOURS(float gamma);
	neighbour *CreateNeighbour(int tmp_mac, float pktime);
	void AddNeighbourPacketToForward(neighbour *neig, char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, int originPort, int destinationPort, double pktime);
	void AddNeighbourPacketForwarded(neighbour *neig, char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, double pktime);
	void AddFlow(int32_t source, int32_t destination, neighbour *nb, double pktime);
	neighbour *ExistNeighbourMac(int mac);
	void AssignIpToNeighbour(int mac, int32_t ip, int addNew, double pktime, int ownMAC);
	void SearchForBlackHole(int percentile_of_loss, int32_t ownIP, int heartbeat_malicious, int immediate_reaction_duration, double pktime, int sufix);
	void SearchForBlackHoleUsingBeta(float percentile_of_loss, int32_t ownIP, int heartbeat_malicious, int immediate_reaction_duration, double pktime, int sufix, int storing_time);
	void DeleteOldNeighbours(int time_max, int debug, double pktime, int ownIP);
	void RemoveOldPackets(int storing_time);
	void RemoveOldFlows(int time_max, double pktime);
	void CompressPacketsAllNeighbours(int reduction);
	float NodesPacketsStored();
	void RemoveOldPackets(int storing_time, double pktime);
	void ShowAllNeighbours(int percentile_of_loss, float pktime);
	void ShowAllInterestingNeighbours(int percentile_of_loss, int ownIP, int ownMAC, float pktime);
	void UpdateBetaOfNeighbourhood(float discount, int storing_time, float pktime, float observation_time);
protected:
private:
	double start_time;
	double max_percentage;
	neighbours *neigs;
	int finish;
	float gamma;

	void AddNeighbour(neighbour *ngb, int verbose, double pktime);
	void DeleteAllNeighbours(neighbours *neigs);
	void IniciateNeighbours(neighbours *neigs);
	int NumberOfFlows(neighbour *nb);
	void RemoveFlow(neighbour *nb, flow *f);
	void RemoveFlowByIP(int32_t sourceIp, int32_t destinationIp, neighbour *nb);
	int ExistFlow(int32_t source, int32_t destination, neighbour *nb, double pktime);
	flow *NewFlow(int32_t source, int32_t destination, double pktime);
	void RemoveOldFlowsForNode(neighbour *nb, int time_max, double pktime);
	void RemoveAllFlowsOfNode(neighbour *nb);
	void UpdateNeighbour(int tmp_mac, double pktime);
	void error(char *msg);
	void CleanNeighbour(neighbour *neig);
	neighbour *ExistNeighbour(neighbour *ngb);
	void DeleteNeighbour(neighbour *nb);
	int IsNeighbourPacketFlow(neighbour *neig, int32_t packet_ip_dst);
	void obtainNameProtocol(char *name, int number);
	void ShowNeighbour(neighbour *nb, int percentile_of_loss, float pktime);
	float ObtainPercentageOfLostInTime(neighbour *n, int lastSeconds);
	void RemoveAllOldBetaPackets(neighbours *neigs, int storing_time);
	void RecalculateAllBeta(neighbours *neigs, float discount);
	bool file_exists(const char *filename);
};

#endif /* __NEIGHBOUR_H__ */

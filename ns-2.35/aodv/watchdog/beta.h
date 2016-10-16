/*
This software is designed by Jorge Hortelano Otero.
softwaremagico@gmail.com
Copyright (C) 2010 Jorge Hortelano Otero.
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
Created on January of 2010.
 */

#ifndef __BETA_H__
#define __BETA_H__

#include"packets.h"
#include"config.h"


struct beta_packets_listened {
	float forwarded_packets; /* Packets forwarded by each protocol. */
	float total_forwarded_packets; /* Packets forwarded by each protocol. */
	float not_forwarded_packets; /* Packets forwarded by each protocol. */
	float total_not_forwarded_packets; /* Packets forwarded by each protocol. */
	int stored_packets;
	float total_packets;
	packet *packets;
	packet *last_packet;  /* Pointer to the last packet inserted */
	float last_time_checked;
};


class BETA {

public:
	float alpha;
	float beta;
	float last_time_checked;
	float last_time_updated;
	BETA(float pktime);
	beta_packets_listened *beta_packets;
	void UpdateBeta(float observation, float discount);
	float ExpectedOfBeta();
	void RecalculateBeta(float discount, int storing_time, float pktime);
	void RemoveOldBetaPackets(int storing_time, float pktime);
	void RemoveAllBetaPackets(beta_packets_listened *pckts);
	packet *BetaExistsPacket(packet *pckts, char *body, int ip_dst, int ip_src, int data_size, int max_time, int protocol, float pktime);
	void AddBetaPacketForwarded(char *body, int ip_dst, int ip_src, int data_size, int storing_time, int protocol, float pktime);
	void AddBetaPacketToForward(char *body, int ip_dst, int ip_src, int data_size, int storing_time, int protocol, int originPort, int destinationPort, float pktime);
	void CreateBetaPackets();
	double beta_function();
	double IntegrationOfBeta(float low_limit, float upper_limit);
	double ShortIntegrationOfBeta(float low_limit, float upper_limit);
	double CumulativeDistributionFunction(float gamma);

protected:
	void RemoveBetaPacket(beta_packets_listened *pckts, packet *pk);
	void ObtainObservation(float discount, int storing_time, float pktime);
	void DeletePacket(packet *p) ;

private:
	double GammaFunction(float x);
	double ProbabilityDensityBeta(float x);
	float BETA_PRECISION;
};



#endif /* __BETA_H__ */

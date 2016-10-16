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


#include"beta.h"
#include"math.h"

/**
 * Initiate the structure.
 */
void
BETA::CreateBetaPackets() {
	//beta_packets_listened *bp;

	beta_packets = (beta_packets_listened *)(malloc(sizeof(beta_packets_listened)));

	beta_packets->packets = NULL;
	beta_packets->last_packet = NULL;
	beta_packets->not_forwarded_packets = 0;
	beta_packets->total_not_forwarded_packets = 0;
	beta_packets->forwarded_packets = 0;
	beta_packets->total_forwarded_packets = 0;
	beta_packets->stored_packets = 0;
	beta_packets->total_packets=0;
}

/**
 * Stores a packet no forwarded yet.
 */
void
BETA::AddBetaPacketToForward(char *body, int ip_dst, int ip_src, int data_size, int storing_time, int protocol, int originPort, int destinationPort, float pktime) {
	packet *pk;
	packet *exists;

	beta_packets->total_packets++;

	/* Sometimes can listen a packet two or more times, we must avoid insert the same packet two times */
	exists = BetaExistsPacket(beta_packets->packets, body, ip_dst, ip_src, data_size, storing_time, protocol, pktime);
	if (exists == NULL) {
		pk = (packet *)malloc(sizeof(packet));
		pk->data = (char *)malloc(sizeof(char) * data_size);
		//memcpy(pk->data, body, data_size);
		strncpy(pk->data, body, data_size);
		pk->ip_src = ip_src;
		pk->ip_dst = ip_dst;
		pk->data_size = data_size;
		pk->originPort = originPort;
		pk->destinationPort = destinationPort;
		pk->protocol = protocol;
		pk->previous = NULL;
		pk->next = NULL;
		pk->devalue = 1;
		pk->time = pktime;
		beta_packets->stored_packets++;

		if (beta_packets->packets == NULL) {
			beta_packets->packets = pk;
			beta_packets->last_packet = pk;
		} else {
			beta_packets->last_packet->next = pk;
			pk->previous = beta_packets->last_packet;
			beta_packets->last_packet = pk;
		}
	}
}

/**
 *A packet has been forwarded
 */
void
BETA::AddBetaPacketForwarded(char *body, int ip_dst, int ip_src, int data_size, int storing_time, int protocol, float pktime) {
	packet *exists;

	exists = BetaExistsPacket(beta_packets->packets, body, ip_dst, ip_src, data_size, storing_time, protocol, pktime);
	if (exists != NULL) {
		RemoveBetaPacket(beta_packets, exists);
		beta_packets->forwarded_packets++;
		beta_packets->total_forwarded_packets++;
	}
}

/**
 * Search for a recently stored packet (Only the last SHORT_TIME seconds are checked).
 */
packet *
BETA::BetaExistsPacket(packet *pckts, char *body, int ip_dst, int ip_src, int data_size, int max_time, int protocol, float pktime) {
	packet *p;

	if (pckts == NULL) return NULL;

	p = pckts;
	while (p != NULL) {
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
 * Free al pointers of a packet.
 */
void
BETA::DeletePacket(packet *p) {
	free(p->data);
	free(p);
}

/**
 * Delete a stored packet.
 */
void
BETA::RemoveBetaPacket(beta_packets_listened *pckts, packet *pk) {
	packet *next, *prev;
	if (pk != NULL) {
		next = pk->next;
		prev = pk->previous;

		/* is the first packet */
		if (pk == pckts->packets) {
			pckts->packets = pk->next;
		}

		/* is the last packet */
		if (next == NULL) {
			pckts->last_packet = pk->previous;
		}

		if (next != NULL) next->previous = prev;
		if (prev != NULL) prev->next = next;

		pckts->stored_packets--;

		DeletePacket(pk);
		pk = NULL;

	} else {
		printf("This packet does not exists!\n");
	}
}

void
BETA::RemoveAllBetaPackets(beta_packets_listened *pckts) {
	packet *p, *tmp_p;

	p = pckts->packets;

	while (p != NULL) {
		tmp_p = p;
		p = p->next;
		RemoveBetaPacket(pckts, tmp_p);
		pckts->not_forwarded_packets++;
		pckts->total_not_forwarded_packets++;
	}
}

/**
 * Search in all neighbour the old packets and delete it.
 */
void
BETA::RemoveOldBetaPackets(int storing_time, float pktime) {
	packet *p=NULL, *tmp_p;

	if (beta_packets!=NULL) {
		p = beta_packets->packets;

		while (p != NULL) {
			if (pktime - p->time > storing_time) {
				tmp_p = p;
				p = p->next;
				RemoveBetaPacket(beta_packets, tmp_p);
				beta_packets->not_forwarded_packets++;
				beta_packets->total_not_forwarded_packets++;
			} else {
				p = p->next;
			}
		}
	}
}

/**
 * Packet forwarded obtained for beta function.
 */
void
BETA::ObtainObservation(float discount, int storing_time, float pktime) {
	float observation;
	if (DEBUGGER>3)printf("ObtainObservation\n");
	if ((beta_packets->forwarded_packets + beta_packets->not_forwarded_packets) > 10) {
		observation = ((beta_packets->forwarded_packets) / (beta_packets->forwarded_packets + beta_packets->not_forwarded_packets));
		beta_packets->forwarded_packets = 0;
		beta_packets->not_forwarded_packets = 0;
		UpdateBeta(observation, discount);
	}
}

/**
 * Search for the packets loss and update beta function according to it.
 */
void
BETA::RecalculateBeta(float discount, int storing_time, float pktime) {
	if (DEBUGGER>3)printf("RecalculateBeta\n");
	RemoveOldBetaPackets(storing_time, pktime);
	ObtainObservation(discount, storing_time, pktime);
}

/**
 * Obtain the expected value of the beta function.
 */
float
BETA::ExpectedOfBeta() {
	if (alpha+beta>0) {
		return alpha / (alpha + beta);
	}
	return 0;
}

/**
 * Change the value of the beta function of a neighbour.
 */
void
BETA::UpdateBeta(float observation, float discount) {
	if (DEBUGGER>3)printf("UpdateBeta\n");
	alpha = discount * alpha + (1 - observation);  	/* packet loss */
	beta = discount * beta + observation;			/* packet forwarded */
}

/**
* Obtain the aproximative integration value of the beta funcion
*/
double
BETA::ShortIntegrationOfBeta(float low_limit, float upper_limit) {
	double beta_short=0.01;
	if (DEBUGGER>3)printf("ShortIntegrationOfBeta\n");
	double i;
	double total = 0;
	i = low_limit;
	i += beta_short;
	while (i < upper_limit + beta_short) {
		total += (ProbabilityDensityBeta(i) * beta_short);
		i += beta_short;
	}
	return total;
}


/**
* Obtain the integration value of the beta funcion
*/
double
BETA::IntegrationOfBeta(float low_limit, float upper_limit) {
	if (DEBUGGER>3)printf("IntegrationOfBeta\n");
	double i;
	double total = 0;
	i = low_limit;
	i += BETA_PRECISION;
	while (i <= upper_limit) {
		total += (ProbabilityDensityBeta(i) * BETA_PRECISION);
		i += BETA_PRECISION;
	}
	return total;
}

/**
* Precission is too bad when alpha > beta * 10, but sure it is a malicious one.
*/
double
BETA::CumulativeDistributionFunction(float gamma) {
	BETA_PRECISION = 0.00001;
	if (beta >= alpha) {
		return 0; /* It is impossible to be a malicious in this case (gamma > 0.5) */
	} else if (alpha > beta * 20) { /* To avoid a lack of precission, we do the opposite side of the function */
		return (1 - IntegrationOfBeta(0, gamma));
	} else {
		/* Short integration for save simulation time */
		if (ShortIntegrationOfBeta(gamma, 1) >= gamma) {
			return IntegrationOfBeta(gamma, 1);
		} else {
			return 0;
		}
	}
}

/**
* Obtain the beta function.
*/
double
BETA::beta_function() {
	if (DEBUGGER>3)printf("beta_function\n");
	if (alpha>0 && beta>0) {
		double num = GammaFunction(alpha) * GammaFunction(beta);
		double det = GammaFunction(alpha + beta);
		if (det != 0) {
			return num/det;
		}
	}
	return 1;
}

double
BETA::ProbabilityDensityBeta(float x) {
	double beta_func;
	if (DEBUGGER>3)printf("ProbabilityDensityBeta\n");
	if (alpha==0) return 0;
	beta_func=beta_function();
	if (beta_func!=0) {
		if ((x < 0) || (x > 1) || (x==0 && alpha <= 1) || (x==1 && beta <= 1)) {  // avoiding pow(0,<=0), pow(-X,0.X) --> indetermination;
			return 0;
		}
		double res = ((pow(x, alpha - 1)) * (pow((1 - x), beta - 1)))/beta_func;
		return res;
	}
	return 0;
}

/**
* Obtain the gamma value for the beta function in the coordinate X.
*/
double
BETA::GammaFunction(float x) {
	if (DEBUGGER>3) printf("GammaFunction of %f\n",x);
	if (x>0) {
		return tgamma(x);
	}
	return 0;
}

/**
Constructor
*/
BETA::BETA(float pktime) {
	CreateBetaPackets();
	alpha= 1;
	beta= 1;
	last_time_updated = pktime;
	last_time_checked = pktime;
}

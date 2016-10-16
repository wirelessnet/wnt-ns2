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

#include "neighbours.h"

bool
NEIGHBOURS::file_exists(const char *filename) {
	if (FILE *file = fopen(filename, "r")) {
		fclose(file);
		return true;
	}
	return false;
}


/**
* Show the error
*/
void NEIGHBOURS::error(char *msg) {
	perror(msg);
	exit(-1);
}


void
NEIGHBOURS::obtainNameProtocol(char *name, int number) {
	if (number == 2) {
		strcpy(name, "UDP");
	}        /* is TCP packet*/
	else if (number == 1) {
		strcpy(name, "TCP");
	}        /* is ICMP packet */
	else if (number == 3) {
		strcpy(name, "AUDIO");
	} else if (number == 4) {
		strcpy(name, "VIDEO");
	} else if (number == 16) {
		strcpy(name, "RTP");
	} else if (number == 30) {
		strcpy(name, "FTP");
	} else
		strcpy(name, "");
}


/**
 * Return the number of stored flows for a node.
 */
int
NEIGHBOURS::NumberOfFlows(neighbour *nb) {
	int number = 0;
	flow *f;
	f = nb->flows;
	while (f != NULL) {
		number++;
		f = f->next;
	}
	return number;
}


/**
 * Remove one selected flow
 */
void
NEIGHBOURS::RemoveFlow(neighbour *nb, flow *f) {
	if (f!=NULL) {
		if (f->previous != NULL) {
			f->previous->next = f->next;
		} else {
			nb->flows = f->next;
		}
		if (f->next != NULL) {
			f->next->previous = f->previous;
		} else {
			if (f->previous != NULL) {
				f->previous->next = NULL;
			} else {
				nb->flows = NULL;
			}
		}
		nb->number_flows--;
		free(f);
	}
}


/**
 * Delete one flow selected by IP
 */
void
NEIGHBOURS::RemoveFlowByIP(int32_t sourceIp, int32_t destinationIp, neighbour *nb) {
	flow *f;

	f = nb->flows;
	while (f != NULL) {
		if ((destinationIp == f->destination) && (sourceIp == f->source)) { /* Remove the flow */
			RemoveFlow(nb, f);
			break;
		}
		f = f->next;
	}
}


/**
 * Return 0 if the IP is not a stored flow of the neighbour nb.
 */
int
NEIGHBOURS::ExistFlow(int32_t source, int32_t destination, neighbour *nb, double pktime) {
	flow *f;
	f = nb->flows;

	while (f != NULL) {
		if ((destination == f->destination) && (source == f->source)) {
			f->time = pktime; /* Update the last time used. */
			return 1;
		}
		f = f->next;
	}
	return 0;
}


/**
 * Reserve the memory for a new flow.
 */
flow*
NEIGHBOURS::NewFlow(int32_t source, int32_t destination, double pktime) {
	flow *f;

	if ((f = (flow *)(malloc(sizeof(flow)))) < 0) {
		error("Memory assignment of flow!\n");
	}
	f->destination = destination;
	f->source = source;
	f->next = NULL;
	f->previous = NULL;
	f->time = pktime;
	return f;
}


/**
 * Add a new flow definded by a IP to a neighbour.
 */
void
NEIGHBOURS::AddFlow(int32_t source, int32_t destination, neighbour *nb, double pktime) {
	flow *f, *ant, *n;
	f = nb->flows;

	ant = NULL;
	if (!ExistFlow(source, destination, nb, pktime)) {
		while (f != NULL) {
			ant = f;
			f = f->next;
		}
		n = NewFlow(source, destination, pktime);
		n->previous = ant;
		if (ant != NULL) {
			ant->next = n;
		} else {
			nb->flows = n;
		}
		nb->number_flows++;
	}
}


/**
 * Delete old flows for one node.
 */
void
NEIGHBOURS::RemoveOldFlowsForNode(neighbour *nb, int time_max, double pktime) {
	flow *f, *bak;
	double time = pktime;
	f = nb->flows;
	while (f != NULL) {
		if (time - f->time > time_max) {
			bak = f;
			f = f->next;
			RemoveFlow(nb, bak);
		} else {
			f = f->next;
		}
	}
}


/**
 * Delete old flows for all nodes.
 */
void
NEIGHBOURS::RemoveOldFlows(int time_max, double pktime) {
	neighbour *n;

	n = neigs->group;
	while (n != NULL) {
		RemoveOldFlowsForNode(n, time_max, pktime);
		n = n->next;
	}
}


void
NEIGHBOURS::RemoveAllFlowsOfNode(neighbour *nb) {
	flow *f, *bak;
	f = nb->flows;
	while (f != NULL) {
		bak = f;
		f = f->next;
		RemoveFlow(nb, bak);
	}
}


/**
 * Update the time of neighbour identified by a mac.
 */
void
NEIGHBOURS::UpdateNeighbour(int tmp_mac, double pktime) {
	neighbour *n;
	n = neigs->group;
	while (n != NULL) {
		if (n->mac == tmp_mac) {
			n->time = pktime;
		}
		n = n->next;
	}
}


/**
 * Determines if a neighbour is already inserted. Return the number of neighbour.
 */
neighbour*
NEIGHBOURS::ExistNeighbourMac(int mac) {
	neighbour *n;
	n = neigs->group;
	while (n != NULL) {
		if (n->mac == mac) {
			return n;
		}
		n = n->next;
	}
	return NULL;
}


/**
 * Determines if a neighbour is already inserted. Return the number of neighbour.
 */
neighbour*
NEIGHBOURS::ExistNeighbour(neighbour *ngb) {
	return ExistNeighbourMac(ngb->mac);
}


/**
 * Add a previously generated neighbour into the visible neighbours.
 */
void
NEIGHBOURS::AddNeighbour(neighbour *ngb, int verbose, double pktime) {
	if (ngb->mac != 666) {
		if (ExistNeighbour(ngb) != NULL) {
			if (verbose)
				printf("Node %d already exists!\n", ngb->mac);
			UpdateNeighbour(ngb->mac, pktime);
		} else {
			ngb->time = pktime;

			if (neigs->last == NULL) {
				neigs->group = ngb;
				neigs->last = ngb;
				ngb->previous = NULL;
			} else {
				neigs->last->next = ngb;
				ngb->previous = neigs->last;
				neigs->last = ngb;
			}
			ngb->starting_time = pktime;
			neigs->howmany++;
			if (verbose)
				printf("Node %d inserted!\n", ngb->mac);
		}
	}
}


/**
 * Inicialize the struct neighbour with a mac
 */
neighbour*
NEIGHBOURS::CreateNeighbour(int tmp_mac, float pktime) {
	neighbour *ngb;
	if ((ngb = (neighbour *)(malloc(sizeof(neighbour)))) < 0)
		error("Memory assignment of neighbour!\n");
	ngb->mac = tmp_mac;
	ngb->ip = -666;
	ngb->pckts = new PACKETS();
	ngb->reputation = new BETA(pktime);
	ngb->malicious_node = -666;
	ngb->malicious_node_bf = -666;
	ngb->immediate_reaction = 0;
	ngb->next = NULL;
	ngb->previous = NULL;
	ngb->flows = NULL;
	ngb->number_flows = 0;
	return ngb;
}


/**
 * Shows the mac and ip of the neighbour stored in a determined index.
 */
void
NEIGHBOURS::ShowNeighbour(neighbour *nb, int percentile_of_loss, float pktime) {
	int i;
	float forward_pcks = 0, received_pcks = 0;
	int vectors[256];
	double packets_fw_vector[256];
	double packets_rv_vector[256];
	float packets_st_vector[256];
	int vectors_detected = 0;
	char protocol_name[MAX_PROTOCOL_NAME];
	char tmp[15];
	char malicious[12];
	flow *f;
	char *text;

	if (nb == NULL) {
		printf("This node not exists.\n");
	} else {
		for (i = 0; i < NUMBER_PROTOCOLS; i++) {
			forward_pcks += nb->pckts->forwarded_packets(i);
			received_pcks += nb->pckts->received_packets(i);
			if (nb->pckts->received_packets(i) > 0) {
				vectors[vectors_detected] = i;
				packets_fw_vector[vectors_detected] = nb->pckts->forwarded_packets(i);
				packets_rv_vector[vectors_detected] = nb->pckts->received_packets(i);
				packets_st_vector[vectors_detected] = nb->pckts->stored_packets_protocol(i);
				vectors_detected++;
			}
		}

		if ((nb->malicious_node > 0) || (nb->malicious_node_bf > 0)) {
			strcpy(malicious, "malicious");
		} else {
			strcpy(malicious, " ");
		}

		/*General information of the node */
		printf("\tNode: IP %d MAC %d (%f secs)\n"
		       "\t\tTotal\tRv:%6.0f\tFw:%6.0f\tSt:%.0f\tBeta:%.2f a:%f b:%f gamma:%f (Rv: %.0f Fw: %.0f (%.0f) Ls:%.0f (%.0f) St: %d)\t%s\n",
		       nb->ip, nb->mac,
		       pktime,
		       received_pcks,
		       forward_pcks,
		       nb->pckts->stored_packets(),
		       nb->reputation->CumulativeDistributionFunction(gamma),
		       nb->reputation->alpha, nb->reputation->beta,
		       gamma,
		       nb->reputation->beta_packets->total_packets,
		       nb->reputation->beta_packets->forwarded_packets,
		       nb->reputation->beta_packets->total_forwarded_packets,
		       nb->reputation->beta_packets->not_forwarded_packets,
		       nb->reputation->beta_packets->total_not_forwarded_packets,
		       nb->reputation->beta_packets->stored_packets,
		       malicious);

		/* Information by type of vector */
		for (i = 0; i < vectors_detected; i++) {
			obtainNameProtocol(protocol_name, vectors[i]);
			printf("\t\t%s\tRv:%6.0f\tFw:%6.0f\tSt:%.0f\n",
			       protocol_name,
			       packets_rv_vector[i],
			       packets_fw_vector[i],
			       packets_st_vector[i]
			      );
		}

		text = (char *) malloc(sizeof(char) * ((40 * NumberOfFlows(nb)) + 30));
		strcpy(text, "\0");
		f = nb->flows;
		if (f != NULL) {
			sprintf(text, "\t\tNode of %d route/s: ", nb->number_flows);
			while (f != NULL) {
				strcat(text, "[");
				sprintf(tmp, "%d", f->source);
				strcat(text, tmp);
				strcat(text, ", ");
				sprintf(tmp, "%d", f->destination);
				strcat(text, tmp);
				strcat(text, "], ");
				f = f->next;
			}
		}
		strcat(text, "\0");
		printf("%s\n", text);
		printf("\n");
		free(text);
	}
}


/**
 * Show the macs and ips of all stored neighbours.
 */
void
NEIGHBOURS::ShowAllNeighbours(int percentile_of_loss, float pktime) {
	if (neigs->howmany > 0) {
		neighbour *n;
		n = neigs->group;
		while (n != NULL) {
			ShowNeighbour(n, percentile_of_loss, pktime);
			n = n->next;
		}
	} else {
		printf("List of neighbour empty!\n");
	}
	printf("(Stored %d neighbours)\n----------\n", neigs->howmany);
}

/**
 * Show the macs and ips of all stored neighbours.
 */
void
NEIGHBOURS::ShowAllInterestingNeighbours(int percentile_of_loss, int ownIP, int ownMAC, float pktime) {
	int show=0;
	if (neigs->howmany > 0) {

		//Something interesting to show?
		neighbour *n;
		n = neigs->group;
		while (n != NULL) {
			if ((n->malicious_node > 0) || (n->malicious_node_bf > 0)) {
				show=1;
				break;
			}

			if (n->reputation->alpha > n->reputation->beta) {
				show=1;
				break;
			}
			n = n->next;
		}

		if (show) {
			printf("Node IP: %d MAC: %d says:\n", ownIP, ownMAC);

			n = neigs->group;
			while (n != NULL) {
				ShowNeighbour(n, percentile_of_loss, pktime);
				n = n->next;
			}
		}
	} else if (show) {
		printf("List of neighbour empty!\n");
	}
	if (show) {
		printf("(Stored %d neighbours)\n----------\n", neigs->howmany);
	}
}


/**
 * Free all pointers of a Neighbour
 */
void
NEIGHBOURS::CleanNeighbour(neighbour *neig) {
	if (neig != NULL) {
		//neig->pckts->DeletePacketsListened(neig->pckts);
		delete neig->pckts;
		RemoveAllFlowsOfNode(neig);
		free(neig);
	}
}


/**
 * Delete a specific neighbour.
 */
void
NEIGHBOURS::DeleteNeighbour(neighbour *nb) {
	neighbour *next, *prev;


	if (nb != NULL) {
		next = nb->next;
		prev = nb->previous;

		/* is the first neighbour */
		if (nb == neigs->group) {
			neigs->group = nb->next;
		}

		/* is the last packet */
		if (nb == neigs->last) {
			neigs->last = nb->previous;
		}

		if (next != NULL) next->previous = prev;
		if (prev != NULL) prev->next = next;

		CleanNeighbour(nb);
		nb = NULL;
		neigs->howmany--;
	} else {
		printf("This neighbour does not exists!\n");
	}
}


/*
 * Delete the neighbours without any recevied packets for a time.
 */
void
NEIGHBOURS::DeleteOldNeighbours(int time_max, int debug, double pktime, int ownIP) {
	double time;
	neighbour *n, *tmp;

	time = pktime;

	n = neigs->group;
	while (n != NULL) {
		tmp = n->next;
		if (time - n->time > time_max) {
			if (debug) printf("\tDeleted in node: %d by timeout: mac (%d) (%.2f secs)\n", ownIP, n->mac, pktime - start_time);
			DeleteNeighbour(n);
		}
		n = tmp;
	}
}


void
NEIGHBOURS::DeleteAllNeighbours(neighbours *neigs) {
	neighbour *n, *tmp;

	n = neigs->group;
	while (n != NULL) {
		tmp = n->next;
		DeleteNeighbour(n);
		n = tmp;
	}
}


/**
 * Is the packet's flow neighbour.
 */
int
NEIGHBOURS::IsNeighbourPacketFlow(neighbour *neig, int32_t packet_ip_dst) {
	if (neig == NULL) {
		return 0;
	}
	if (neig->ip == packet_ip_dst) return 0;
	return 1;
}


/**
 * Update a neighbour with a mac. If addNew is true, add the neighbour if not exist.
 */
void
NEIGHBOURS::AssignIpToNeighbour(int mac, int32_t ip, int addNew, double pktime, int ownMAC) {
	neighbour *ng;
	if (mac != ownMAC && mac != 0 && ip != 0) { /* I am not my neighbour and is a valid IP and MAC */
		if ((ng = ExistNeighbourMac(mac)) == NULL) {
			if (addNew && mac != 0) {
				ng = CreateNeighbour(mac, pktime);
				ng->ip = ip;
				AddNeighbour(ng, 0, pktime);
			}
		} else {
			ng->ip = ip;
			ng->time = pktime;
		}
	}
}


/**
 * Add a packet to a neighbour not forwarded yet.
 */
void
NEIGHBOURS::AddNeighbourPacketToForward(neighbour *neig, char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, int originPort, int destinationPort, double pktime) {
	neig->time = pktime;
	neig->pckts->AddPacketToForward(body, ip_dst, ip_src, data_size, storing_time, protocol, originPort, destinationPort, pktime);
	neig->reputation->AddBetaPacketToForward(body, ip_dst, ip_src, data_size, storing_time, protocol, originPort, destinationPort, pktime);

}


/**
 * Add a packet to a neighbour not forwarded yet.
 */
void
NEIGHBOURS::AddNeighbourPacketForwarded(neighbour *neig, char *body, int32_t ip_dst, int32_t ip_src, int data_size, int storing_time, int protocol, double pktime) {
	neig->time = pktime;
	neig->pckts->AddPacketForwarded(body, ip_dst, ip_src, data_size, storing_time, protocol, pktime);
	neig->reputation->AddBetaPacketForwarded(body, ip_dst, ip_src, data_size, storing_time, protocol, pktime);
}


/**
 * Search for a black hole node.
 */
void
NEIGHBOURS::SearchForBlackHole(int percentile_of_loss, int32_t ownIP, int heartbeat_malicious, int immediate_reaction_duration, double pktime, int sufix) {
	int j;
	char text[250];
	char protocol_name[MAX_PROTOCOL_NAME];
	neighbour *n;
	double percentage;
	FILE *f;
	char path[250];

	n = neigs->group;

	while (n != NULL) {
		/* Porcentaje dividido por protocolos y puertos */
		for (j = 0; j < NUMBER_PROTOCOLS; j++) {
			if (n->pckts->received_packets(j) > MIN_SIGNIFICANT_PACKETS) {
				percentage = ((1 - ((n->pckts->forwarded_packets(j) + n->pckts->stored_packets_protocol(j)) / (n->pckts->received_packets(j)))) * 100);
			} else {
				percentage = -1;
			}
			if (percentage > max_percentage) {
				max_percentage = percentage;
			}

			if ((percentage > percentile_of_loss && (pktime > n->malicious_node + heartbeat_malicious))) {
				obtainNameProtocol(protocol_name, j);
				sprintf(text, "Node %d says: Alarm! Alarm! node %d (mac %d) not forward more than %d%% packets: %.2f%% loss for %s, %.2f secs of execution and %.2f secs from neighbour detection\n",
				        ownIP, n->ip, n->mac, percentile_of_loss, percentage, protocol_name, pktime - start_time, pktime - n->starting_time);
				n->malicious_node = pktime;
				//SendAnAlarm(text, n, ownIP, gw_ip, port, finish);
				//printf("%s", text);

				sprintf(path, "attackers.txt", "", sufix, sufix);
				if (file_exists(path)) {
					f = fopen(path, "a");
					fprintf(f, "%s", text);
					fclose(f);
				}

				if (finish) {
					exit(1);
				}
			}
			if ((percentage > percentile_of_loss && (immediate_reaction_duration > 0) && (pktime > n->malicious_node + immediate_reaction_duration))) {
				//immediateReaction(n->mac, immediate_reaction_duration);
			}
		}
		n = n->next;
	}
}


/**
 * Search for a black hole node.
 */
void
NEIGHBOURS::SearchForBlackHoleUsingBeta(float percentile_of_loss, int32_t ownIP, int heartbeat_malicious, int immediate_reaction_duration, double pktime, int sufix, int storing_time) {
	char text[250];
	neighbour *n;
	FILE *f;
	char path[250];
	double integration_beta;

	n = neigs->group;

	while (n != NULL) {
		if (pktime - n->reputation->last_time_checked > storing_time) {
			n->reputation->last_time_checked = pktime;
			if ((pktime > n->malicious_node_bf + heartbeat_malicious)) {
				integration_beta = n->reputation->CumulativeDistributionFunction(gamma);;
				if ((integration_beta >= gamma) || (n->reputation->alpha > n->reputation->beta*50)) { //Precission is too bad when alpha > beta * 50, but sure it is a malicious one.
					sprintf(text, "Node %d says: Alarm! Alarm! node %d (mac %d) not pass the bayesian filtering at %.2f secs of execution and %.2f secs from neighbour detection using bayesian filtering (Value:%f a:%.4f b:%.4f).\n",
					        ownIP, n->ip, n->mac, pktime - start_time, pktime - n->starting_time, integration_beta, n->reputation->alpha, n->reputation->beta);
					n->malicious_node_bf = pktime;
					sprintf(path, "attackers.txt", "", sufix, sufix);
					if (file_exists(path)) {
						f = fopen(path, "a");
						fprintf(f, "%s", text);
						fclose(f);
					}

					if (finish) {
						exit(1);
					}
				}
			}
		}
		n = n->next;
	}
}


/**
 * Search in all neighbour the old packets and delete it.
 */
void
NEIGHBOURS::RemoveOldPackets(int storing_time, double pktime) {
	int k, i;
	packet *p, *tmp_p;
	neighbour *n;

	n = neigs->group;
	while (n != NULL) {
		for (k = 0; k < NUMBER_PROTOCOLS; k++) {
			p = n->pckts->get_packet(k);
			i = 0;
			while (p != NULL) {
				i++;
				if (pktime - p->time > storing_time) {
					tmp_p = p;
					p = p->next;
					n->pckts->RemovePacket(tmp_p, k);
					continue;
				}
				p = p->next;
			}
		}
		n = n->next;
	}
}


/**
 * return total packets stored
 */
float
NEIGHBOURS::NodesPacketsStored() {
	float total = 0;
	neighbour *n;
	n = neigs->group;
	while (n != NULL) {
		total += n->pckts->stored_packets();
		n = n->next;
	}
	return total;
}


/**
 * Reduce the number of forwarded packets to avoid long time at false negatives.
 */
void
NEIGHBOURS::CompressPacketsAllNeighbours(int reduction) {
	neighbour *n;

	n = neigs->group;
	while (n != NULL) {
		n->pckts->CompressPackets(reduction);
		n = n->next;
	}
}

/**
* Upgrade beta funcion of all the neighbourhood.
*/
void
NEIGHBOURS::UpdateBetaOfNeighbourhood(float discount, int storing_time, float pktime, float observation_time) {
	neighbour *n;

	n = neigs->group;
	while (n != NULL) {
		if (pktime - n->reputation->last_time_updated >= observation_time) { /* Avoid to update beta on each packet received (Observation time). */
			n->reputation->RecalculateBeta(discount, storing_time, pktime);
			n->reputation->last_time_updated = pktime;
		}
		n = n->next;
	}
}




/*
Constructor
*/
NEIGHBOURS::NEIGHBOURS(float tmp_gamma) {
	finish = FINISH;
	neigs = (neighbours *)malloc(sizeof(neighbours));
	max_percentage = 0;
	neigs->howmany = 0;
	neigs->group = NULL;
	neigs->last = NULL;
	gamma = tmp_gamma;
}

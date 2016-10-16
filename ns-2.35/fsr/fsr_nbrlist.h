/***************************************************************************
                    fsr_nbrlist.h  -  FSR Neighbor List
                             -------------------
    purpose              : Implementation of a Fisheye State Routing (FSR)
                            module for ns-2 (vers. 2.27)
    reference            : draft-ietf-manet-fsr-03.txt
    begin                : Mon Aug 2 2004
    copyright            : (C) 2004 by Sven Jaap
      			   Institute of Operating Systems
    			   and Computer Networks
    			   Technical University of Braunschweig
    			   Germany
email                : jaap@ibr.cs.tu-bs.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 #ifndef __fsr_nbrlist_h__
 #define __fsr_nbrlist_h__

 #include <lib/bsd-list.h>
 #include <sys/types.h>
 #include <assert.h>
 #include <scheduler.h>
 #include <config.h>
 #include <fsr/fsr_rtable.h>

 /**
    Entry of the neighbor list
 **/

class FSR_Neighbor {
    friend class FSR_Agent;
    friend class fsr_tt_entry;
public:
    FSR_Neighbor(u_int32_t a) {nbr_addr = a;}
protected:
    LIST_ENTRY(FSR_Neighbor)  nbr_link;
    nsaddr_t                  nbr_addr;             // Address of neighbored node
    double                    nbr_lastRecvTime;     // Last time a packet was received from neighbor
};

LIST_HEAD(fsr_nbrlist, FSR_Neighbor);
#endif /* __fsr_nblist_h__ */

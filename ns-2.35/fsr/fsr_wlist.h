/***************************************************************************
                    fsr_wlist.h  -  FSR Iteration List for findSP()
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
 #ifndef __fsr_wlist_h__
 #define __fsr_wlist_h__

 #include <lib/bsd-list.h>
 #include <sys/types.h>
 #include <assert.h>
 #include <scheduler.h>
 #include <config.h>
 #include <fsr/fsr_rtable.h>
 #include <fsr/fsr_ttable.h>
 /**
    Entry of the iteration list
 **/

class fsr_wlist_entry {
    friend class FSR_Agent;
    friend class fsr_tt_entry;
    friend class fsr_rt_entry;
public:
    fsr_wlist_entry(u_int32_t a, fsr_tt_entry* tt_ent, fsr_rt_entry* rt_ent){
        node_addr = a;
        tt=tt_ent;
        rt=rt_ent;
    }
protected:
    LIST_ENTRY(fsr_wlist_entry)  node_link;
    nsaddr_t                     node_addr;             // Address of the node
    fsr_tt_entry* tt;                                   // corresponding Topo Table entry
    fsr_rt_entry* rt;                                   // corresponding Routing Table entry
};

LIST_HEAD(fsr_wlist, fsr_wlist_entry);
#endif /* __fsr_wlist_h__ */

/***************************************************************************
              fsr_ttable.h  -  Header File of FSR Topology Table
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

 #ifndef __fsr_ttable_h__
 #define __fsr_ttable_h__

 #include <lib/bsd-list.h>
 #include <sys/types.h>
 #include <assert.h>
 #include <scheduler.h>
 #include <config.h>
 #include <fsr/fsr_nbrlist.h>
 #define NOW  Scheduler::instance().clock()



 /**
    Entry of the topology table
 **/

 class fsr_tt_entry {
        friend class fsr_ttable;
        friend class FSR_Agent;
 public:
           fsr_tt_entry();
           ~fsr_tt_entry();

           /*
             Management of destination's neighbors
           */
           void           nbr_insert(nsaddr_t addr);
           FSR_Neighbor*  nbr_lookup(nsaddr_t addr);
           int            nbr_size(void);             //returns number of neighbors
           void           nbr_clear(void);            //clears list of neigbors
 protected:
           nsaddr_t       tt_dest;        //address of destination node
           u_int32_t      tt_destseq;     //destination sequence number
           double         tt_lstheardtime;//last heard time
           fsr_nbrlist    tt_nbrlist;     //list of destination's neighbors
           int            nbrlist_size;   //contains number of node's neighbors
           u_int32_t      tt_prevseq;     //previous sequence number
           bool           tt_needtosend;  // "NeedToSend" Flag

           LIST_ENTRY(fsr_tt_entry) tt_link;
 };

 /**
    Representation of the FSR topology table
 **/

 class fsr_ttable {
  friend class FSR_Agent;
  public:
          fsr_ttable()  { LIST_INIT(&tthead); }

          fsr_tt_entry* head() { return tthead.lh_first; }  // returns head of table (list)
          fsr_tt_entry* tt_add(nsaddr_t addr);              // add new entry to table
          void          tt_delete(nsaddr_t addr);           // remove entry from table
          fsr_tt_entry* tt_lookup(nsaddr_t addr);           // lookup entry in table
  private:
          LIST_HEAD(fsr_tthead, fsr_tt_entry) tthead;
 };

#endif /* __fsr_ttable_h__ */

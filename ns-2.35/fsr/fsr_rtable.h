/***************************************************************************
              fsr_rtable.h  -  Header File of FSR Routing Table
                             -------------------
    purpose              : Implementation of a Fisheye State Routing (FSR)
                            module for ns-2 (vers. 2.27)
    reference            : draft-ietf-manet-fsr-03.txt
    begin                : Thu Jul 29 2004
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

 #ifndef __fsr_rtable_h__
 #define __fsr_rtable_h__

 #include <lib/bsd-list.h>
 #include <sys/types.h>
 #include <assert.h>
 #include <scheduler.h>
 #include <config.h>

 #define NOW  Scheduler::instance().clock()
 #define UNREACHABLE2 0xff


 /**
    Entry of the routing table
 **/

 class fsr_rt_entry {
        friend class fsr_rtable;
        friend class FSR_Agent;
 public:
           fsr_rt_entry();
 protected:
           nsaddr_t   rt_dest;    //address of destination node
           nsaddr_t   rt_nexthop; //address of the next hop node
           u_int16_t  rt_dist;    //distance to the destination (hops)

           LIST_ENTRY(fsr_rt_entry) rt_link;
 };

 /**
    Representation of the FSR routing table
 **/

 class fsr_rtable {
        friend class FSR_Agent;
  public:
          fsr_rtable()  { LIST_INIT(&rthead); }

          fsr_rt_entry* head() { return rthead.lh_first; } 	//returns head of table (list)
          fsr_rt_entry* rt_add(nsaddr_t addr);        	    // add entry to table
          void          rt_delete(nsaddr_t addr);        	// remove entry form table
          void          rt_clear(nsaddr_t my_addr);     	// remove all entries except own
          void          rt_purge(void);             		// remove unreachable dest from table
          fsr_rt_entry* rt_lookup(nsaddr_t addr);   	 	// lookup entry in table
  private:
          LIST_HEAD(fsr_rthead, fsr_rt_entry) rthead;
 };

#endif /* __fsr_rtable_h__ */

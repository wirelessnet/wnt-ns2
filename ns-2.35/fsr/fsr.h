/***************************************************************************
                  fsr.h  -  Header File of FSR_Agent/Timers
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

#ifndef __fsr_h_
#define __fsr_h_

#include <priqueue.h>
#include <cmu-trace.h>
#include <fsr/fsr_rtable.h>
#include <fsr/fsr_ttable.h>
#include <fsr/fsr_nbrlist.h>
#include <fsr/fsr_wlist.h>
#include <classifier/classifier-port.h>
#include <scheduler.h>

#define MAX_QUEUE_LENGTH      5
#define ROUTER_PORT           0xff

#define FSR_UNREACHABLE       0xFFFF

//Weight of a link (here: 1, but might be complex function)
#define FSR_WEIGHT            1

//Jitter to avoid synchronization
#define LS_UPDATE_JITTER      0.1
#define FSR_BROADCAST_JITTER  0.01

//Parameters of the fisheye scopes
#define NUM_OF_SCOPES         2
#define SCOPE1_INTERVAL       5       //[sec]
#define SCOPE2_INTERVAL       15      //[sec]
#define SCOPE1_HOPS           2       //[hops]
#define SCOPE2_HOPS           0xFFFF  //[hops]

//Multiplier for Topology Table timeout
#define TOPO_TIMEOUT_MUL      3     // Timeout[sec] = x * SCOPE(i)_INTERVAL

// Timeout interval for neighbored nodes
#define NEIGHBOR_TIMEOUT      15  //[sec]
// Max. number of considered neighbors per node
//   => limited by field length in LS Update Msg
#define FSR_MAX_NBRS         255  //[nodes] (255==8 Bit)
// Max. Packet Size for LSUpdate
#define FSR_MAX_PKT_SIZE     65535// [bytes] (65535==16 Bit)

#define NOW Scheduler::instance().clock()

class FSR_Agent;

/*************************************************************
    TIMERS
**************************************************************/

/*
  Timer for FSR Link State Updates (Scope 1)
*/
class LSUpdateTimerS1: public Handler {
public:
        LSUpdateTimerS1(FSR_Agent* a) : agent(a) {}
        void handle(Event*);
        void init();
private:
        FSR_Agent   *agent;
        Event       intr;

};

#ifdef SCOPE2_INTERVAL
/*
  Timer for FSR Link State Updates (Scope 2)
*/
class LSUpdateTimerS2: public Handler {
public:
        LSUpdateTimerS2(FSR_Agent* a) : agent(a) {}
        void handle(Event*);
        void init();
private:
        FSR_Agent   *agent;
        Event       intr;

};
#endif

#ifdef SCOPE3_INTERVAL
/*
  Timer for FSR Link State Updates (Scope 3)
*/
class LSUpdateTimerS3: public Handler {
public:
        LSUpdateTimerS3(FSR_Agent* a) : agent(a) {}
        void handle(Event*);
        void init();
private:
        FSR_Agent   *agent;
        Event       intr;

};
#endif
/*
  Timer for removing stale entries from the neigbor list
*/
class NbrPurgeTimer: public Handler {
public:
        NbrPurgeTimer(FSR_Agent* a) : agent(a) {}
        void handle(Event*);
private:
        FSR_Agent   *agent;
        Event       intr;

};
/*

/*************************************************************
  FSR Routing Agent
**************************************************************/
class FSR_Agent : public Agent {
    friend class fsr_rt_entry;
    friend class fsr_tt_entry;
    friend class LSUpdateTimerS1;
    friend class LSUpdateTimerS2;
    friend class LSUpdateTimerS3;
    friend class NbrPurgeTimer;
    friend class TopoPurgeTimer;
public:
    FSR_Agent(nsaddr_t id);
    void    recv(Packet *p, Handler *);
protected:
    int     command(int argc, const char * const * argv);
    int     initialized() {return 1 && target_;}
    void    initialize_node(void);
    /*
       Packet Transmission Methods
    */
    void    sendLSUpdate(int lowerBound, int upperBound);       //send new LS Update
    void    forward(fsr_rt_entry *rt,Packet *p, double delay);  //forward packet
    /*
      Packet Reception Methods
    */
    void            recvLSUpdate(Packet *p);
    /*
      Variables
    */
    Trace           *tracetarget;    // Trace Target
    PriQueue        *ifqueue;        // link level output queue
    u_int32_t       curr_seqno;      // Sequence number to advertise with...
    nsaddr_t        my_addr;         // node's own address
    bool            nbrsChanged;     // flag to determine changes in the neighbor list
    bool            topoChanged;     // flag to determine changes in the topo table
    /*
      FSR LISTS
    */
    fsr_nbrlist     nbrhead;           // FSR neighbor list
    fsr_wlist       whead;             // Iterationlist for FSR_Agent::findSP()
    /*
      Management of the FSR neighbor list
    */
    void            nbr_insert(nsaddr_t addr); //insert new neighbor into list
    void            nbr_delete(nsaddr_t addr); //delete neighbor from list
    int             nbr_size(void);            //get number of neighbors
    void            nbr_purge(void);           //delete expired neighbors
    FSR_Neighbor*   nbr_lookup(nsaddr_t addr); //get neighbor from list
    int             nbrlist_size;              //contains current number of neighbors
    /*
      Management of the findSP() iteration list
    */
    void            w_insert(nsaddr_t addr, fsr_tt_entry*, fsr_rt_entry*);   //insert new node into list
    void            w_delete(nsaddr_t addr);   //delete node from list
    void            w_clear(void);             //remove all nodes from list
    int             w_size(void);              //get number of nodes
    fsr_wlist_entry*   w_lookup(nsaddr_t addr);   //get node from list
    int             wlist_size;                //contains current size of list
    /*
      TABLES
    */
    fsr_ttable      ttable;
    fsr_rtable      rtable;
    /*
      TOPO TABLE MANAGEMENT
    */
    void            tt_addOwnEntry(void);       //add own entry to Topology Table
    void            tt_updateOwnEntry(void);    //update own entry in Topo Table
    void            tt_purge(void);             //remove stale entries from topo table
    /*
      RT TABLE MANAGEMENT
    */
    void               rt_addOwnEntry(void);                   //add own entry to Routing Table
    void               rt_initnbrnode(fsr_rt_entry*,nsaddr_t); //init entry for neighbored node
    void               rt_initothernode(fsr_rt_entry*);        //init entry for other node
    void               rt_replace(void);                       //calculate new RT Table
    void               rt_findSP(void);                        //create shortest path tree
    void               rt_resolve(Packet *p);                  //resolve route to destination
    fsr_wlist_entry*   rt_getnxtentry(void);                   //get next entry for calculation in findSP()
    /*
      TIMERS
    */

    LSUpdateTimerS1 lstimerS1;
    #ifdef SCOPE2_INTERVAL
    LSUpdateTimerS2 lstimerS2;
    #endif
    #ifdef SCOPE3_INTERVAL
    LSUpdateTimerS3 lstimerS3;
    #endif
    NbrPurgeTimer nbrtimer;
    PortClassifier *port_dmux_;    // port dmux of wireless ns-2 node
    /*
      METHODS FOR DEBUGGING PURPOSES
    */
    void            show_nbrlist(void);
    void            show_ttable(void);
    void            show_rtable(void);
    void            show_wlist(void);
};
#endif

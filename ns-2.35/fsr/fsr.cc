/***************************************************************************
                       fsr.cc  -  FSR Routing Agent
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


#include <fsr/fsr.h>
#include <random.h>
#include <address.h>

/*
   TCL Hook for FSR Routing Agent
*/
static class FSRClass:public TclClass{
public:
    FSRClass ():TclClass ("Agent/FSR"){}
    TclObject *create (int argc, const char *const * argv){
        assert(argc==5);
        //create new routing agent
        return (new FSR_Agent ( (nsaddr_t) Address::instance().str2addr(argv[4])));
    }
} class_fsr;

/*
  Initialisation of the Routing Agent when the simulation starts
*/
void
FSR_Agent::initialize_node(){
    nbrsChanged=false;
    topoChanged=false;
    nbrlist_size=0;
    wlist_size=0;
    //initialize Topo Table
    tt_addOwnEntry();
    //initialize Routing Table
    rt_addOwnEntry();
    //Start timers for LS Updates
    lstimerS1.init();
  #ifdef SCOPE2_INTERVAL
    lstimerS2.init();
  #endif
  #ifdef SCOPE3_INTERVAL
    lstimerS3.init();
  #endif
    // Start neighbor timer
    nbrtimer.handle((Event*) 0);
}

/*
   TCL Hook (execution of TCL commands)
*/
int
FSR_Agent::command(int argc, const char*const* argv){
    if (argc == 2) {
        Tcl& tcl=Tcl::instance();
        if (strncasecmp (argv[1], "start-fsr",2) == 0){
            // start routing agent
            initialize_node();
            return (TCL_OK);
        }else if (strncasecmp (argv[1], "id",2) == 0){
            //get own address
            tcl.resultf("%d",my_addr);
            return (TCL_OK);
        }
    }else if (argc == 3){
        if (strcmp(argv[1],"my_addr") == 0){
              //set own address
              my_addr=atoi(argv[2]);
              return TCL_OK;
        }else if (strcasecmp (argv[1], "tracetarget") == 0){
              //set target gor traces
              tracetarget=(Trace*) TclObject::lookup(argv[2]);
              if (tracetarget==0){
                  return TCL_ERROR;
              }else{
                  return TCL_OK;
              }
        }else if (strcasecmp (argv[1], "drop-target") == 0) {
              //
        }else if (strcasecmp (argv[1], "if-queue") == 0) {
              //set reference to interface queue (see node structure in ns-2)
              ifqueue = (PriQueue*) TclObject::lookup(argv[2]);
              if (ifqueue == 0){
                  return TCL_ERROR;
              } else {
                  return TCL_OK;
              }                
        }else if (strcasecmp (argv[1], "port-dmux") == 0) {
              //set reference to port dmux (see node structure in ns-2)
              port_dmux_ = (PortClassifier *)TclObject::lookup(argv[2]);
              if (port_dmux_ == 0){
                  fprintf(stderr, "%s: %s lookup of %s failed\n",__FILE__, argv[1], argv[2]);
                  return TCL_ERROR;
              } else {
                  return TCL_OK;
              }
        }
    }
    return Agent::command (argc, argv);
}

/*
    Constructor of FSR routing agent
*/


FSR_Agent::FSR_Agent(nsaddr_t id): Agent(PT_MESSAGE),
                                     lstimerS1(this),
                          #ifdef SCOPE2_INTERVAL
                                     lstimerS2(this),
                          #endif                                     
                          #ifdef SCOPE3_INTERVAL
                                     lstimerS3(this),
                          #endif
                                     nbrtimer(this){
    // set some variables
    my_addr=id;
    curr_seqno=0;

    LIST_INIT(&nbrhead);
    LIST_INIT(&whead);
    ifqueue=0;
    tracetarget=0;
}

/*************************************************
    TIMERS
**************************************************/

/*
  Handle Event for new LS Update (Scope 1)
*/
void
LSUpdateTimerS1::handle(Event*){
    //send LS update
    agent->sendLSUpdate(0,SCOPE1_HOPS);
    double interval=SCOPE1_INTERVAL+(LS_UPDATE_JITTER * Random::uniform());
    assert (interval >= 0);
    //reschedule event
    Scheduler::instance().schedule(this, &intr, interval);
}

/*
  Initilize Update Timer for scope 1
*/
void
LSUpdateTimerS1::init(){
    //start updates immediately
    handle( (Event*) 0);
}

#ifdef SCOPE2_INTERVAL
/*
  Handle Event for new LS Update (Scope 2)
*/
void
LSUpdateTimerS2::handle(Event*){
    //send LS update  
    agent->sendLSUpdate(SCOPE1_HOPS+1,SCOPE2_HOPS);
    double interval=SCOPE2_INTERVAL+(LS_UPDATE_JITTER * Random::uniform());
    assert (interval >= 0);
    //reschedule event
    Scheduler::instance().schedule(this, &intr, interval);
}
/*
  Initilize Update Timer for scope 2
*/
void
LSUpdateTimerS2::init(){
    //send first updates at about time SCOPE2_INTERVAL
   double interval=SCOPE2_INTERVAL+(LS_UPDATE_JITTER * Random::uniform());
   assert (interval >= 0);
   //schedule event
   Scheduler::instance().schedule(this, &intr, interval);
}
#endif

#ifdef SCOPE3_INTERVAL
/*
  Handle Event for new LS Update (Scope 3)
*/
void
LSUpdateTimerS3::handle(Event*){
 //send LS Update
 agent->sendLSUpdate(SCOPE2_HOPS+1,SCOPE3_HOPS);
 double interval=SCOPE3_INTERVAL+(LS_UPDATE_JITTER * Random::uniform());
 assert (interval >= 0);
 //reschedule event
 Scheduler::instance().schedule(this, &intr, interval);
}
/*
  Initilize Update Timer for scope 2
*/
void
LSUpdateTimerS3::init(){
   //send first updates at about time SCOPE3_INTERVAL
  double interval=SCOPE3_INTERVAL+(LS_UPDATE_JITTER * Random::uniform());
  assert (interval >= 0);
  //schedule event
  Scheduler::instance().schedule(this, &intr, interval);
}
# endif


/*
  Handle Event for purging neighbor list
*/
void
NbrPurgeTimer::handle(Event *){
    agent->nbr_purge();
    Scheduler::instance().schedule(this, &intr,NEIGHBOR_TIMEOUT);
}

/*************************************************
    MANAGEMENT OF THE FSR NEIGHBOR LIST
**************************************************/

/*
    Insert new neighboring node into list
*/
void
FSR_Agent::nbr_insert(nsaddr_t addr){
    FSR_Neighbor *nbr = new FSR_Neighbor(addr);
    assert (nbr); //check whether new neighbor is created
    nbr->nbr_lastRecvTime=NOW;
    LIST_INSERT_HEAD(&nbrhead,nbr,nbr_link);
    nbrlist_size++;
}
/*
    Remove neighboring node from list
*/
void
FSR_Agent::nbr_delete(nsaddr_t addr){
    FSR_Neighbor *nbr = nbrhead.lh_first;
    for(;nbr;nbr=nbr->nbr_link.le_next){
        if(nbr->nbr_addr == addr){
            LIST_REMOVE(nbr,nbr_link);
            delete nbr;
            nbrlist_size--;
            break;
        }
    }
}
/*
    Get number of neighbors
*/
int
FSR_Agent::nbr_size(){
    return nbrlist_size;
}
/*
    Remove neighbors that have expired
*/
void
FSR_Agent::nbr_purge(){
    FSR_Neighbor *nbr=nbrhead.lh_first;
    FSR_Neighbor *next_nbr;
    double now = NOW;
    double interval=NEIGHBOR_TIMEOUT;
    //run through the neighbor list and delete expired entries
    for(;nbr;nbr=next_nbr){
        next_nbr=nbr->nbr_link.le_next;
        if((nbr->nbr_lastRecvTime+interval) < now){
            nbr_delete(nbr->nbr_addr); // remove
            nbrsChanged=true;
        }
    }
}
/*
    Get Neighbor from List
*/
FSR_Neighbor*
FSR_Agent::nbr_lookup(nsaddr_t addr){
    FSR_Neighbor *nbr = nbrhead.lh_first;
    for(;nbr;nbr=nbr->nbr_link.le_next){
        if(nbr->nbr_addr == addr)
          break;
    }
    return nbr;
}

/*************************************************
    MANAGEMENT OF THE findSP() iteration list
**************************************************/

/*
    Insert new node into list
*/
void
FSR_Agent::w_insert(nsaddr_t addr, fsr_tt_entry* tt, fsr_rt_entry* rt){
    fsr_wlist_entry *node = new fsr_wlist_entry(addr,tt,rt);
    assert (node); //check whether new entry is created
    LIST_INSERT_HEAD(&whead,node,node_link);
    wlist_size++;
}

/*
    Remove node from list
*/
void
FSR_Agent::w_delete(nsaddr_t addr){
    fsr_wlist_entry *node = whead.lh_first;
    for(;node;node=node->node_link.le_next){
        if(node->node_addr == addr){
            LIST_REMOVE(node,node_link);
            delete node;
            wlist_size--;
            break;
        }
    }
}

/*
    Get number of nodes
*/
int
FSR_Agent::w_size(){
    return wlist_size;
}

/*
    Remove all elements from list
*/

void
FSR_Agent::w_clear(){
    fsr_wlist_entry *node;
    while(node=whead.lh_first){
        LIST_REMOVE(node,node_link);
        delete node;
    }
    wlist_size=0;
}

/*
    Get Entry from List
*/
fsr_wlist_entry*
FSR_Agent::w_lookup(nsaddr_t addr){
    fsr_wlist_entry *node = whead.lh_first;
    for(;node;node=node->node_link.le_next){
        if(node->node_addr == addr)
          break;
    }
    return node;
}


/*************************************************
    MANAGEMENT OF THE TOPOLOGY TABLE
**************************************************/

/*
    Add own entry to Topology Table after initialization
*/
void
FSR_Agent::tt_addOwnEntry(){
    fsr_tt_entry *tt;
    tt=ttable.tt_lookup(my_addr);
    if(tt==0){
        //set up own entry
        tt=ttable.tt_add(my_addr);
        tt->tt_destseq=curr_seqno;
        tt->tt_lstheardtime=NOW;
        tt->tt_prevseq=curr_seqno;
    }
}
/*
    Update entry of own node in the Topology Table
*/
void
FSR_Agent::tt_updateOwnEntry(){
    FSR_Neighbor *nbr=nbrhead.lh_first;
    if (nbrsChanged){
        int numNbrs=0;
        //increment own sequence number
        curr_seqno++;
        fsr_tt_entry *tt=ttable.tt_lookup(my_addr);
        assert(tt);
        tt->tt_destseq=curr_seqno;
        tt->tt_lstheardtime=NOW;
        tt->tt_needtosend=true;
        tt->nbr_clear();
        //renew own Neighbor list in Topology Table
        for(;nbr && (numNbrs < FSR_MAX_NBRS);nbr=nbr-> nbr_link.le_next){
             tt->nbr_insert(nbr->nbr_addr);
             numNbrs++;
        }
        nbrsChanged=false;
    }
}
/*
    Remove stale entries from the Topology Table
    The Timeout interval depends on the scope's update interval
*/
void
FSR_Agent::tt_purge(){
    fsr_tt_entry *tt = ttable.head();
    fsr_tt_entry *next_tt;
    fsr_rt_entry *rt;
    double scope1_int= ((double)TOPO_TIMEOUT_MUL)*SCOPE1_INTERVAL;
    #ifdef SCOPE2_INTERVAL
    double scope2_int=((double)TOPO_TIMEOUT_MUL)*SCOPE2_INTERVAL;
    #endif
    #ifdef SCOPE3_INTERVAL
    double scope3_int=((double)TOPO_TIMEOUT_MUL)*SCOPE3_INTERVAL;
    #endif
    //iterate through all entries
    for(;tt;tt=next_tt){
        next_tt=tt->tt_link.le_next;
        if(rt=rtable.rt_lookup(tt->tt_dest)){
            // determine node's scope
            // never time out own entry (dist=0)
            if ( 1 <= rt->rt_dist && rt->rt_dist <= SCOPE1_HOPS){
                //SCOPE 1
                if ( (tt->tt_lstheardtime + scope1_int) < NOW){
                    ttable.tt_delete(tt->tt_dest);
                    continue;
                }
            }
         #ifdef SCOPE2_INTERVAL
            else if ((SCOPE1_HOPS < rt->rt_dist) && (rt->rt_dist <= SCOPE2_HOPS)){
                //SCOPE 2
                if ( (tt->tt_lstheardtime + scope2_int) < NOW){
                    ttable.tt_delete(tt->tt_dest);
                    continue;
                }
            }
         #endif
         #ifdef SCOPE3_INTERVAL
            else if ((SCOPE2_HOPS < rt->rt_dist) && (rt->rt_dist <= SCOPE3_HOPS)){
                //SCOPE 3
                if ( (tt->tt_lstheardtime + scope3_int) < NOW){
                    ttable.tt_delete(tt->tt_dest);
                    continue;
                }
            }
         #endif
        }
    }
}

/*************************************************
    MANAGEMENT OF THE ROUTING TABLE
**************************************************/

/*
    Add own entry to Topology Table after initialization
*/
void
FSR_Agent::rt_addOwnEntry(){
    fsr_rt_entry *rt;
    rt=rtable.rt_lookup(my_addr);
    if(rt==0){
        //set up own entry
        rt=rtable.rt_add(my_addr);
        rt->rt_dist=0;
        rt->rt_nexthop=my_addr;
    }
}
/*
    initialize neighbored node
*/
void
FSR_Agent::rt_initnbrnode(fsr_rt_entry* rt, nsaddr_t dest){
    rt->rt_dist=FSR_WEIGHT;
    rt->rt_nexthop=dest;
}

/*
    initialize other node
*/
void
FSR_Agent::rt_initothernode(fsr_rt_entry* rt){
    rt->rt_dist=FSR_UNREACHABLE;
    rt->rt_nexthop=-1;
}

/*
    get entry for node from the routing table that is
    currently not processed (is in w list) and is
    closest to me
*/
fsr_wlist_entry*
FSR_Agent::rt_getnxtentry(){
    fsr_wlist_entry *min;
    fsr_wlist_entry *w=whead.lh_first;
    int minDist=FSR_UNREACHABLE;
    //iterate through w_list
    for(;w;w=w->node_link.le_next){
        if (w->rt->rt_dist <= minDist){
            //found new node with min distance to me => remember it
            minDist=w->rt->rt_dist;
            min=w;
        }
    }
    return min;
}
/*
   Start update of routing table
*/
void
FSR_Agent::rt_replace(){
    // 1. Purge Topology Table
    tt_purge();
    // 2. Clear current Routing Table
    rtable.rt_clear(my_addr);
    // 3. find shortest path tree rooted at my node
    rt_findSP();
    // 4. remove entries that are unreachable
    //    (may appear e.g. due to timeouts of topo entries )
    rtable.rt_purge();
}

/*
   Determining the routing table from the known topology information
        => calculate shortest path tree routed at my node
*/
void
FSR_Agent::rt_findSP(){
    fsr_tt_entry* tt=ttable.head();
    fsr_rt_entry* rt;
    fsr_rt_entry* rt_it;
    fsr_wlist_entry* wl;
    FSR_Neighbor *nbr;
    // 1. INITIALISATION
    // initialisation of own entry
    //     => insert my address in iteration list
    w_clear();
    // initialize the temporary routing table with all known destinations
    for(;tt;tt=tt->tt_link.le_next){
        if (tt->tt_dest==my_addr){
            //Skip own entry
        } else if (nbr_lookup(tt->tt_dest)){
            // destination is in the immediate communication range
            if(!w_lookup(tt->tt_dest)){
                if (!rtable.rt_lookup(tt->tt_dest)){
                    rt=rtable.rt_add(tt->tt_dest);
                    rt_initnbrnode(rt,tt->tt_dest);
                }
                w_insert(tt->tt_dest,tt,rt);
            }
        } else {
                // other nodes that are not immediate neighbors to me
                if(!w_lookup(tt->tt_dest)){
                    if (!rtable.rt_lookup(tt->tt_dest)){
                        rt=rtable.rt_add(tt->tt_dest);
                        rt_initothernode(rt);
                    }
                    w_insert(tt->tt_dest,tt,rt);
                }
          }
        //scan neighbors for further nodes
        nbr=tt->tt_nbrlist.lh_first;
        for (;nbr;nbr=nbr->nbr_link.le_next){
            if (nbr->nbr_addr==my_addr){
                //Skip own entry
            } else if (nbr_lookup(nbr->nbr_addr)){
                // destination is in the immediate communication range
                if (!rtable.rt_lookup(nbr->nbr_addr)){
                   rt=rtable.rt_add(nbr->nbr_addr);
                   rt_initnbrnode(rt,nbr->nbr_addr);
                }
            } else{
                // other nodes that are not immediate neighbors to me
                if (!rtable.rt_lookup(nbr->nbr_addr)){
                      rt=rtable.rt_add(nbr->nbr_addr);
                      rt_initothernode(rt);
                }
            }
        }
    }
    // 2. START CALCULATION
    while (whead.lh_first){
        // iterate until all nodes from w list are processed
        wl=rt_getnxtentry();
        // Get pointers to entries in Topo and Routing Table
        tt=wl->tt;
        rt=wl->rt;
        if (tt){
            nbr=tt->tt_nbrlist.lh_first;
            for (;nbr;nbr=nbr->nbr_link.le_next){
                // check routes to all neighbors from rt->dest
                rt_it=rtable.rt_lookup(nbr->nbr_addr);
                if( (rt->rt_dist+FSR_WEIGHT) < rt_it->rt_dist){
                    // found new shortest path
                    rt_it->rt_dist=rt->rt_dist+FSR_WEIGHT;
                    rt_it->rt_nexthop=rt->rt_nexthop;
                }
            }
        }
        w_delete(wl->node_addr);
    }
}

/*
    Resolve route to destination and forward packet
*/
void
FSR_Agent::rt_resolve(Packet *p){
    struct hdr_ip *iph = HDR_IP(p);
    struct hdr_cmn *cmh = HDR_CMN(p);
    fsr_rt_entry *rt;
    // lookup route to destination
    rt=rtable.rt_lookup(iph->daddr());
    if (rt && (rt->rt_dist < FSR_UNREACHABLE) &&(rt->rt_dest!=-1)) {
        // found entry in routing Table
        forward(rt,p,-1.0);                   //forward packet immediately
    } else {
        // no route available
        drop(p,DROP_RTR_NO_ROUTE);
    }
}
/*************************************************
    METHODS CONCERNING THE RECEPTION OF PACKETS
**************************************************/

/*
   Main Packet Reception Routine
*/
void
FSR_Agent::recv (Packet * p, Handler *)
{
    struct hdr_ip *iph = HDR_IP(p);
    struct hdr_cmn *cmh = HDR_CMN(p);
    assert(initialized());
    /*
     *  Must be a packet I'm originating...
     */
    if((iph->saddr() == my_addr)&& (cmh->num_forwards() == 0) ) {
        /*
         * Add the IP Header
         */
        cmh->size() += IP_HDR_LEN;
        if ( (u_int32_t)iph->daddr() != IP_BROADCAST ){
            iph->ttl_ = IP_DEF_TTL;
        }
    }
    /*
     *  I received a packet that I sent.  Probably
     *  a routing loop.
     */
    else if(iph->saddr() == my_addr) {
        drop(p, DROP_RTR_ROUTE_LOOP);
        return;
    }
    /*
       Received a Link State Update
    */
    else if ((iph->saddr() != my_addr) && (iph->dport() == ROUTER_PORT)&& (iph->sport() == ROUTER_PORT)){
        iph->ttl_ -=1;
        recvLSUpdate(p);
        return;
    }
    /*
     *  Packet I'm forwarding...
     */
    else {
        /*
         *  Check the TTL.  If it is zero, then discard.
         */
        if(--iph->ttl_ == 0) {
            drop(p, DROP_RTR_TTL);
            return;
        }
    }
    if ( (u_int32_t) iph->daddr() != IP_BROADCAST){
        rt_resolve(p);                    // search route to destination
        return;
    } else{
        //Broadcast Packet
        forward((fsr_rt_entry*)0,p,-1.0); //forward packet immediately
        return;
    }
}

/*
  Process a LS Updated that was received
*/
void
FSR_Agent::recvLSUpdate(Packet *p){
    struct hdr_ip *iph = HDR_IP(p);
    unsigned char *pktDataPtr;
    FSR_Neighbor *nbr;
    fsr_tt_entry *tt;
    nsaddr_t destAddr;
    nsaddr_t nbAddr;
    u_int32_t destseq;
    int numOfNodes;
    bool newDst;
    //Sanity Checks
    assert(iph->sport() == ROUTER_PORT);
    assert(iph->dport() == ROUTER_PORT);
    //Check whether sender is known neighbor
    nbr=nbr_lookup(iph->saddr());
    if(nbr==0){
        //new neighbor => insert into list
        nbr_insert(iph->saddr());
        // set Flag for Update of Own Entry in Topology Table
        nbrsChanged=true;
    } else {
        //knwon neighbor => update lastRecvTime
        nbr->nbr_lastRecvTime=NOW;
    }
    // Get LS Information from packet
    // 1. Packet Length
    pktDataPtr=p->accessdata();
    int pktLength= *(pktDataPtr++);
    pktLength= pktLength << 8 | *(pktDataPtr++);
    //Skip Reserved (16 Bit)
    pktDataPtr+=2;
    pktLength-=4;
    while(pktLength > 0){
        // Destination Address (32 Bit)
        destAddr = *(pktDataPtr++);
        destAddr = destAddr << 8 | *(pktDataPtr++);
        destAddr = destAddr << 8 | *(pktDataPtr++);
        destAddr = destAddr << 8 | *(pktDataPtr++);
        //DEBUG
        //fprintf(stderr,"Dest: %d \n",destAddr);
        //search destination in Topo Table
        tt=ttable.tt_lookup(destAddr);
        if (tt==0){
          // 1. Case: new destination => add new entry in Topo Table
          tt=ttable.tt_add(destAddr);
          newDst=true;
          topoChanged=true;// remember that Topo Table changed
        }
        // Destination Sequence Number (24 Bit)
        destseq = 0;
        destseq = *(pktDataPtr++);
        destseq = destseq << 8 | *(pktDataPtr++);
        destseq = destseq << 8 | *(pktDataPtr++);
        if ((destseq > tt->tt_destseq) || newDst){
            // 2. Case: a) received most up to date information
            //              => replace current Topo Table entry
            //          b) fill entry for new destination
            newDst=false;
            topoChanged=true; // remember that Topo Table changed
            tt->tt_destseq=destseq;
            tt->tt_lstheardtime=NOW;
            tt->tt_needtosend=true;
            // Number of Nodes (8 Bit)
            numOfNodes=*(pktDataPtr++);
            pktLength-=8;
            tt->nbr_clear();
            for(;numOfNodes>0;numOfNodes--){
                // Neighbor Address (32Bit)
                nbAddr =*(pktDataPtr++);
                nbAddr= nbAddr << 8 | *(pktDataPtr++);
                nbAddr= nbAddr << 8 | *(pktDataPtr++);
                nbAddr= nbAddr << 8 | *(pktDataPtr++);
                tt->nbr_insert(nbAddr);
                pktLength-=4;
            }
        } else if (destseq < tt->tt_prevseq){
            // 3. Case: received outdated information
            //              => send known info with next LS Update
            tt->tt_needtosend=true;
            tt->tt_lstheardtime=NOW;
            numOfNodes=(*(pktDataPtr++))*4;
            // Skip Neigbored Nodes
            pktDataPtr+=numOfNodes;
            pktLength-=(8+(4*numOfNodes));
        } else {
            //nothing to do
            //most up to date info is already known
            tt->tt_lstheardtime=NOW;
            numOfNodes=(*(pktDataPtr++))*4;
            // Skip Neigbored Nodes
            pktDataPtr+=numOfNodes;
            pktLength-=(8+numOfNodes);
        }
    }
    Packet::free(p); // free routing packet
    if (topoChanged) {
        //RECOMPUTE ROUTING TABLE IF TOPOLOGY TABLE CHANGED
        rt_replace();
        topoChanged=false;
    }
}


/*************************************************
    METHODS CONCERNING THE TRANSMISISION OF PACKETS
**************************************************/

/*
    Forward data packet
*/
void
FSR_Agent::forward(fsr_rt_entry *rt,Packet *p, double delay){
    struct hdr_cmn *cmh = HDR_CMN(p);
    struct hdr_ip  *iph = HDR_IP(p);
    //check TTL
    if(iph->ttl_ == 0){
        //drop packet
        drop(p,DROP_RTR_TTL);
        return;
    } else {
        if(cmh->ptype() !=PT_MESSAGE && cmh->direction()==hdr_cmn::UP && ((u_int32_t) iph->daddr() == IP_BROADCAST) || ((u_int32_t)iph->daddr() == here_.addr_)){
                port_dmux_->recv(p,0);  // deliver broadcast packet or packet to me
                return;
        }else{
            if(rt){
                // route to destination is available
                cmh->next_hop_    = rt->rt_nexthop;   //add next hop info to packet
                cmh->addr_type()  = NS_AF_INET;
                cmh->direction()  = hdr_cmn::DOWN;    // change direction of packet
                // FORWARD PACKET
                if (delay > 0.0){
                    Scheduler::instance().schedule(target_,p,delay);
                } else {
                    Scheduler::instance().schedule(target_,p,0.0);
                }
            } else {
                // packet is broadcast packet
                assert(iph->daddr() = (nsaddr_t) IP_BRAODCAST);
                cmh->addr_type()    = NS_AF_NONE;
                cmh->direction()    = hdr_cmn::DOWN;  // change direction od packet
                // FORWARD BROADCAST PACKET
                // use jitter to avoid synchonization
                Scheduler::instance().schedule(target_,p,FSR_BROADCAST_JITTER*Random::uniform());          
            }
        }
    }
}

/*
    Create and send a LS Update to all neighbors
*/
void
FSR_Agent::sendLSUpdate(int lowerBound, int upperBound){
    unsigned char *pktDataPtr;
    nsaddr_t sizeLimitAddr=-1;
    bool     sizeLimit=false;
    //counter for necessary data amount of update
    // initilized with 4 (PacketLength+Reserved);
    int dataAmount=4;
    int tmpAmount=0;
    fsr_tt_entry *tt;
    fsr_rt_entry *rt;
    FSR_Neighbor *nbr;
    Packet *p = Packet::alloc();
    struct hdr_ip  *iph = HDR_IP(p);
    struct hdr_cmn *cmh = HDR_CMN(p);
    //fill packet headers
    cmh->next_hop_    = IP_BROADCAST;
    cmh->addr_type()  = NS_AF_NONE;
    cmh->ptype()      = PT_MESSAGE;
    iph->saddr()      = my_addr;
    iph->daddr()      = IP_BROADCAST;
    iph->sport()      = RT_PORT;
    iph->dport()      = RT_PORT;
    iph->ttl_         = 1;
    if (lowerBound==0){
        //Update own entry first when it belongs to current scope
        tt_updateOwnEntry();
    }
    // remove stale entries from topology table
    tt_purge();
    //Determine amount of Data to be send
    tt=ttable.head();
    for(;tt;tt=tt->tt_link.le_next){
        if(tt->tt_needtosend){
            if (rt=rtable.rt_lookup(tt->tt_dest)){
                if (rt->rt_dist >= lowerBound && rt->rt_dist<=upperBound){
                    tmpAmount=8;//(Dst Addr + DST Seq Nr + num Neighbors)
                    tmpAmount+=(tt->nbr_size()*4); //32bit per Neighbor
                    sizeLimitAddr=tt->tt_dest;
                    if ((dataAmount+tmpAmount) > FSR_MAX_PKT_SIZE){
                        sizeLimit=true;
                        break;
                    } else {
                        dataAmount+=tmpAmount;
                    }
                }
            }
        }
    }
    //set size of packet

    p->allocdata(dataAmount);
    pktDataPtr = p->accessdata();

    //bulid LS Update byte by byte
    // 1. Packet Length (16 Bit)
    u_int16_t pktLength=dataAmount;
    *(pktDataPtr++)= (pktLength >> 8) & 0xFF;
    *(pktDataPtr++)= (pktLength >> 0) & 0xFF;
    // 2. Reserved  (16 Bit)
    *(pktDataPtr++) = 0xFF;
    *(pktDataPtr++) = 0xFF;
    tt=ttable.head();
    for(;tt;tt=tt->tt_link.le_next){
     if (sizeLimit && sizeLimitAddr==tt->tt_dest){
       //break building of Update Msg here due to
       //limitation of packet size in FSR
       break;
     } else {
       if (tt->tt_needtosend){
         if (rt=rtable.rt_lookup(tt->tt_dest)){
           if (rt->rt_dist >= lowerBound && rt->rt_dist<=upperBound){
               // Destination Address (32 Bit)
               *(pktDataPtr++)= (tt->tt_dest >> 24) & 0xFF;  
               *(pktDataPtr++)= (tt->tt_dest >> 16) & 0xFF;
               *(pktDataPtr++)= (tt->tt_dest >> 8) & 0xFF;
               *(pktDataPtr++)= (tt->tt_dest >> 0) & 0xFF;
               // Destination Sequence Number (24 Bit)
               // stored in u_int32_t
               *(pktDataPtr++)= (tt->tt_destseq >> 16) & 0xFF;
               *(pktDataPtr++)= (tt->tt_destseq >> 8) & 0xFF;
               *(pktDataPtr++)= (tt->tt_destseq >> 0) & 0xFF;
               // Number of Neighbors (8 Bit)
               *(pktDataPtr++)=tt->nbr_size();
               FSR_Neighbor *nbr=tt->tt_nbrlist.lh_first;
               for(;nbr;nbr=nbr->nbr_link.le_next){
                   //Neighbor Address (32 Bit)
                   *(pktDataPtr++)= (nbr->nbr_addr >> 24) & 0xFF;
                   *(pktDataPtr++)= (nbr->nbr_addr >> 16) & 0xFF;
                   *(pktDataPtr++)= (nbr->nbr_addr >> 8) & 0xFF;
                   *(pktDataPtr++)= (nbr->nbr_addr >> 0) & 0xFF;
               }
               //reset flag
               tt->tt_needtosend=false;
               //replace prev. seq.-nr. with curr. seq.-nr
               tt->tt_prevseq=tt->tt_destseq;
           }
         }
       }
     }
    }
    //set Size of Packet
    cmh->size()      = IP_HDR_LEN+dataAmount;
    Scheduler::instance().schedule(target_,p,LS_UPDATE_JITTER*Random::uniform());
    // create further packet when limit of packet lenght was reached
    if (sizeLimit){
        sendLSUpdate(lowerBound,upperBound);
    }
}

/*************************************************
    METHODS FOR DEBUGGING PUROPSES
**************************************************/
/*
    show entries of FSR Neighbor List
*/
void
FSR_Agent::show_nbrlist(){
    FSR_Neighbor *nbr=nbrhead.lh_first;
    printf("\nNeighbors of: %d\n",my_addr);
    printf("-------------\n");
    for(;nbr;nbr=nbr->nbr_link.le_next){
        printf("NB: %d TIME: %f\n",nbr->nbr_addr,nbr->nbr_lastRecvTime);
    }
    printf("Current Size: %d",nbrlist_size);
}

/*
    show entries of FSR Topology Table
*/
void
FSR_Agent::show_ttable(){
    fsr_tt_entry *tt=ttable.tthead.lh_first;
    printf("\nTopo Info of: %d\n",my_addr);
    printf("-------------\n");
    for(;tt;tt=tt->tt_link.le_next){
        printf("ID: %d SEQ: %d TIME: %f\n",tt->tt_dest,tt->tt_destseq,tt->tt_lstheardtime);
    }
}

/*
    show entries of FSR Routing Table
*/
void
FSR_Agent::show_rtable(){
    fsr_rt_entry *rt=rtable.head();
    printf("\nRT Info of: %d\n",my_addr);
    printf("-------------\n");
    for( ; rt ; rt=rt->rt_link.le_next){
        printf("ID: %d DIST: %d NXT: %d\n",rt->rt_dest,rt->rt_dist,rt->rt_nexthop);
    }
}

/*
    show entries of findSP() iteration list
*/
void
FSR_Agent::show_wlist(){
    fsr_wlist_entry *node=whead.lh_first;
    printf("W LIST of: %d\n",my_addr);
    printf("-------------\n");
    for(;node;node=node->node_link.le_next){
        printf("ID: %d \n",node->node_addr);
    }
}

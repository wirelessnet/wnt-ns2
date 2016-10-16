/***************************************************************************
                    fsr_ttable.cc  -  FSR Topology Table
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

 #include <fsr/fsr_ttable.h>
 
/***************************************
  Enry of the FSR topology table
***************************************/

/*
  Constructor
*/
fsr_tt_entry::fsr_tt_entry(){
    //Initialize entry of Topo Table
    tt_dest=0;
    tt_destseq=0;
    tt_lstheardtime=0;
    tt_prevseq=0;
    tt_needtosend=false;
    LIST_INIT(&tt_nbrlist);
    nbrlist_size=0;
}
/*
  Destructor
*/
fsr_tt_entry::~fsr_tt_entry(){
    FSR_Neighbor *nbr;
    //remove all entries from the neighbor list
    while((nbr=tt_nbrlist.lh_first)){
        LIST_REMOVE(nbr,nbr_link);
        delete nbr;
    }
}
/********************************************
   MANAGEMENT OF A DESTINATION's NEIGHBORS
********************************************/

/*
  Insert new neighbor
*/
void
fsr_tt_entry::nbr_insert(nsaddr_t addr){
    FSR_Neighbor *nbr = new FSR_Neighbor(addr);
    assert(nbr);
    nbr->nbr_lastRecvTime=0; //not needed in Topo Table
    LIST_INSERT_HEAD(&tt_nbrlist, nbr, nbr_link);
    nbrlist_size++;
}
/*
  Get neighbor from List
*/
FSR_Neighbor*
fsr_tt_entry::nbr_lookup(nsaddr_t addr){
 FSR_Neighbor *nbr = tt_nbrlist.lh_first;
        for(;nbr;nbr=nbr->nbr_link.le_next){
            if(nbr->nbr_addr==addr){
                break;
            }
        }
    return nbr;
}
/*
    Get number of neighbors
*/
int
fsr_tt_entry::nbr_size(){
    return nbrlist_size;
}
/*
    Clears the list of neighbors
*/
void
fsr_tt_entry::nbr_clear(){
    FSR_Neighbor *nbr;
    //remove all entries from the neighbor list
    while((nbr=tt_nbrlist.lh_first)){
        LIST_REMOVE(nbr,nbr_link);
        delete nbr;
    }
    nbrlist_size=0;
}

/***************************************
   METHODS OF FSR_TTABLE
***************************************/

/****
  Lookup entry of specific destination in table
*****/
fsr_tt_entry*
fsr_ttable::tt_lookup(nsaddr_t addr)
{
  fsr_tt_entry *entry = tthead.lh_first;
  for (;entry;entry = entry->tt_link.le_next){
      if (entry->tt_dest == addr){
         break;
      }
  }
  return entry;
}

/****
  Delete entry of specific destination in table
*****/
void
fsr_ttable::tt_delete(nsaddr_t addr)
{
  fsr_tt_entry *entry = tt_lookup(addr);
  if (entry){
    LIST_REMOVE(entry, tt_link);
    delete entry;
  }
}

/****
  Add entry to the table
*****/
fsr_tt_entry*
fsr_ttable::tt_add(nsaddr_t addr)
{
  fsr_tt_entry *entry;
  //ensure that destination is unknown so far
  entry = new fsr_tt_entry;
  assert(entry);
  entry->tt_dest=addr;
  entry->tt_needtosend=true;
  LIST_INSERT_HEAD(&tthead, entry, tt_link);
  return entry;
}

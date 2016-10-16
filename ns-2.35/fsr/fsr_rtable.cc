/***************************************************************************
                    fsr_rtable.cc  -  FSR Routing Table
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

 #include <fsr/fsr_rtable.h>

/*********************
  Enry of the FSR routing table

**********************/ 

fsr_rt_entry::fsr_rt_entry(){


}   

/************************************
   METHODS OF FSR_TTABLE
*************************************/

/****
  Lookup entry of specific destination in table
*****/
fsr_rt_entry*
fsr_rtable::rt_lookup(nsaddr_t addr)
{
  fsr_rt_entry *entry = rthead.lh_first;
  for (;entry;entry = entry->rt_link.le_next){
      if (entry->rt_dest == addr){
         break;
      }
  }
  return entry;
}

/****
  Delete entry of specific destination in table
*****/
void
fsr_rtable::rt_delete(nsaddr_t addr)
{
  fsr_rt_entry *entry = rt_lookup(addr);
  if (entry){
    LIST_REMOVE(entry, rt_link);
    delete entry;
  }
}

/****
  Remove all entries from routing table (except own)
*****/
void
fsr_rtable::rt_clear(nsaddr_t my_addr)
{
  fsr_rt_entry *entry;
  while((entry = rthead.lh_first)){
    LIST_REMOVE(entry, rt_link);
    delete entry;
  }
  //rtable must always contain entry of own node
  entry=rt_add(my_addr);
  entry->rt_dist=0;
  entry->rt_nexthop=my_addr;
}

/****
  Remove unreachable destinations from table
*****/
void
fsr_rtable::rt_purge()
{
  fsr_rt_entry *entry = rthead.lh_first;
  fsr_rt_entry *next_entry;
  for (;entry;entry = next_entry){
      next_entry=entry->rt_link.le_next;
      if (entry->rt_nexthop == -1){
        //Destination is unreachable => remove it
        rt_delete(entry->rt_dest);
      }
  }
}


/****
  Add entry to the table
*****/
fsr_rt_entry*
fsr_rtable::rt_add(nsaddr_t addr)
{
  fsr_rt_entry *entry;
  entry = new fsr_rt_entry;
  assert(entry);
  entry->rt_dest=addr;
  LIST_INSERT_HEAD(&rthead, entry, rt_link);
  return entry;
}

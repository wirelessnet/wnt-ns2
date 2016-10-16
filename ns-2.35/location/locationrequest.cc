/************************************************************************************
 *	Copyright (c) 2012 CSIR.
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions
 *	are met:
 *		1. Redistributions of source code must retain the above copyright
 *			notice, this list of conditions and the following disclaimer.
 *		2. Redistributions in binary form must reproduce the above copyright
 *			notice, this list of conditions and the following disclaimer in the
 *			documentation and/or other materials provided with the distribution.
 *		3. All advertising materials mentioning features or use of this software
 *			must display the following acknowledgement:
 *
 *				This product includes software developed by the Advanced Sensor
 *				Networks Group at CSIR Meraka Institute.
 *
 *		4. Neither the name of the CSIR nor of the Meraka Institute may be used
 *			to endorse or promote products derived from this software without
 *			specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY CSIR MERAKA INSTITUTE ``AS IS'' AND
 *	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *	ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *	OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *	OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *	SUCH DAMAGE.
 ************************************************************************************
 *
 *		File: ~/ns-allinone-2.34/ns-2.34/location/locationrequest.cc	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: This file defines the "LocReqAgent" class, which is derived 
 *						from the Agent class. This agent is responsible for 
 *						constructing a "location request" packet (PT_LOCREQ) and then
 *						broadcasting it to neighbouring nodes.
 ************************************************************************************/


#include "locationrequest.h"
#include "locationpacket.h"
 
// Binding C++ and OTcl classes
int hdr_locreq::offset_;

static class LocReqHeaderClass : public PacketHeaderClass {
	public:
		LocReqHeaderClass() : PacketHeaderClass("PacketHeader/LocReq", sizeof(hdr_locreq)) {
		bind_offset(&(hdr_locreq::offset_));
	}
} class_locreqhdr;

static class LocReqAgentClass : public TclClass {
	public:
		LocReqAgentClass() : TclClass("Agent/LocReq"){}
		TclObject* create(int, const char*const*) {
			return (new LocReqAgent() );
		}
} class_locreq_agent;

// Constructor
LocReqAgent::LocReqAgent() :	Agent(PT_LOCREQ), ldapp_(0)
{
	bind("packetSize_", &size_);
}

LocReqAgent::LocReqAgent(packet_t type) :	Agent(type)
{
	bind("packetSize_", &size_);
}
// Distructor
LocReqAgent::~LocReqAgent()
{
	delete ldapp_;
}

// Attach the application with the agent
void LocReqAgent::attachApp(LocDisApp* app)
{
	ldapp_ = ( LocDisApp*) app;
}
// Construct a "location request" packet (PT_LOCREQ) that will be broadcasted to neighbouring nodes
void LocReqAgent::broadcast()
{
	Packet *p = Packet::alloc();
	hdr_cmn *ch = HDR_CMN(p);
	hdr_ip *iph = HDR_IP(p);
 
	ch->ptype() = PT_LOCREQ;
	ch->next_hop_ = IP_BROADCAST;

	iph->saddr() = Agent::addr();
	iph->daddr() = IP_BROADCAST;
	iph->sport() = RT_PORT;
	iph->dport() = RT_PORT;
	iph->ttl_ = 1;

	// send packet to the link layer
	send(p, 0);
}

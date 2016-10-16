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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/locationresponse.cc	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: This class is a child of class Agent. It is responsible for
 *						 receiving packets. This agent should be attached to all nodes
 *						 (unknowns and beacons). Two types of packets could be received.
 *						 The first is a location request packet (PT_LOCREQ). If this
 *						 type of packet is received by a beacon or reference node it
 *						 constructs a “location response” packet (PT_LOCRES) that
 *						 includes location information and then sends it to the
 *						 requesting node. Unknown nodes receiving this type of packet
 *						 simply deallocate it. The second is a location response packet
 *						 (PT_LOCRES). The requesting node that receives this packet sends
 *						 it to the application layer (LocDisApp), which processes the
 *						 included location information to estimate the node's position.
 ************************************************************************************/

#include "locationresponse.h"
#include "locationpacket.h"

// Binding C++ and OTcl classes

int hdr_locres::offset_;

static class LocResHeaderClass : public PacketHeaderClass {
	public:
		LocResHeaderClass() : PacketHeaderClass("PacketHeader/LocRes", sizeof(hdr_locres)) {
		bind_offset(&(hdr_locres::offset_));
	}
} class_locreshdr;

static class LocResAgentClass : public TclClass {
public:
    LocResAgentClass() : TclClass("Agent/LocRes") {}
    TclObject* create(int, const char*const*) {
        return (new LocResAgent());
    }
} class_locres_agent;


// Constructor
LocResAgent::LocResAgent() : Agent(PT_LOCRES), ldapp_(0), set_target_(false)
{
	bind("packetSize_", &size_);
}

LocResAgent::LocResAgent(packet_t type) : Agent(type)
{
	bind("packetSize_", &size_);
}

// Distructor
LocResAgent::~LocResAgent()
{
	delete [] ldapp_;
}

// Attach the application with the agent
void LocResAgent::attachApp(LocDisApp* app)
{
	ldapp_ = (LocDisApp*) app;
}

// Two types of packets could be received; either request or response packets
void LocResAgent::recv(Packet* p, Handler*)
{
	hdr_cmn* ch = HDR_CMN(p);

	if (ch->ptype() == PT_LOCREQ)
		send_res(p, 0);
	else if (ch->ptype() == PT_LOCRES)
		recv_res(p, 0);
		
	Packet::free(p); 
}

// Send a location response packet that include the the location information of the sending node
void LocResAgent::send_res(Packet* p, Handler*)
{
	if(!ldapp_)
		return;

	// Unknown nodes do not sent response
	if (ldapp_->get_node_att() == UNKNOWN)
		return;
	
	Packet* npkt = Packet::alloc();
	if (!npkt)
		return;

	hdr_ip* oiph = HDR_IP(p);

	hdr_cmn *ch = HDR_CMN(npkt);
	hdr_ip* niph = HDR_IP(npkt);
	hdr_locres* lres = HDR_LOCRES(npkt);

	ch->ptype() = PT_LOCRES;
	
	niph->saddr() = Agent::addr();
	niph->sport() = LOCRES_PORT;
	niph->daddr() = oiph->src_.addr_;
	niph->dport() = LOCRES_PORT;
	niph->ttl_ = 1;

	lres->loc()	= ldapp_->get_node_loc();
	
	// Set link layer as target
	if(!set_target_)
	{
		target_ = get_link_layer();
		set_target_ = true;
	}

	double delay_;
	delay_ = Random::uniform(0.0, 0.5);

	//Send the response packet after a delay 
	Scheduler::instance().schedule(target_, npkt, delay_);
}

// Send the received response packet to the location discovery application for further processing
void LocResAgent::recv_res(Packet* p, Handler*)
{
	if(!ldapp_)
		return;

	// BEACON nodes drop response packets
	if (ldapp_->get_node_att() == BEACON)
		return;

	hdr_cmn* ch = hdr_cmn::access(p);
	// The location discovery application will process the received packet
	ldapp_->process_response(ch->size(), p);
}

// Obtain the C++ compiled object for the link layer
NsObject* LocResAgent::get_link_layer()
{
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s set node_", this->name());
	tcl.evalf("%s set ll_(0)",tcl.result());
	return (NsObject *)TclObject::lookup(tcl.result());
}

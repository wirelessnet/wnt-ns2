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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/locationdiscovery.cc	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: This class is derived from the Application class. Each node in
 *						 the network uses an object from this class by attaching it to
 *						 its agent(s). The LocDisApp class performs several functions,
 *						 such as periodically invoking the broadcast method of LocReqAgent
 *						 to broadcast a "location request" packet, processing the received
 *						 “location response” packet and estimating the node location.
 ************************************************************************************/

#include "locationdiscovery.h"
#include "locationpacket.h"
#include "refineposition.h"

// Binding C++ and OTcl classes
static class LocDisAppClass : public TclClass {
  public:
    LocDisAppClass() : TclClass("Application/LocDiscovery") {}
    TclObject* create(int, const char*const*) {
      return (new LocDisApp);
    }
} class_locdis_app;

// Constructor
LocDisApp::LocDisApp(): agent_request_(0), agent_response_(0), req_timer_(this), est_timer_(this), output_timer_(this),
								node_location_(), start_time_(0), loc_error_(-1), color_changed_(false), final_(false),
								 node_attribute_(0),	requesting_(0), reqno_(0), no_ref_(0)
								
{
	bind("distanceError_", &distance_error_);
	bind("random_", &random_);
	bind("reqFreq_", &req_freq_);
	bind("maxRequests_", &max_requests_);
	bind("showColor_", &show_color_);
	bind("method_", &method_);
}

// Distructor
LocDisApp::~LocDisApp() 
	{
		delete agent_request_;
		delete agent_response_;
		delete position_;
		delete mobile_node_;
	}

// command function allows the access of the compiled hierarchy from the interpreted hierarchy.
// here it is used to invoke the attach-agent command
// attach-agent is used to make a connection between LocDisApp and the agents (LocReqAgent and LocResAgent)
int LocDisApp::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();

	// Attach agent to the application
	if (argc == 3)
	{
		if (strcmp(argv[1], "attach-agent") == 0)
		{
			agent_ = (Agent*) TclObject::lookup(argv[2]);
			if (agent_ == 0)
			{
				tcl.resultf("no such agent %s", argv[2]);
				return(TCL_ERROR);
			}
			if (agent_->get_pkttype() == PT_LOCREQ)
			{
				agent_request_ = (LocReqAgent*) agent_;
				agent_request_->attachApp(this);
			}

			else if (agent_->get_pkttype() == PT_LOCRES)
			{
				agent_response_ = (LocResAgent*) agent_;
				agent_response_->attachApp(this);
			}				
			else
			{
				tcl.resultf("The agent %s type is neither PT_LOCREQ nor PT_LOCRES", argv[2]);
				return(TCL_ERROR);
			}
			return(TCL_OK);
		}
	}
	return (Application::command(argc, argv));
}

// This function is used to start the LocDisApp, which can be invoked from the OTcl domain
void LocDisApp::start()
{
	init();
	req_timer_.sched(start_time_);
	output_timer_.sched(0.0);
}

// this function is for initialisation purpose, which set the initial values of several variables
// also change the colour of the nodes based on their attribute
void LocDisApp::init()
{
	if(agent_)
	{
		Tcl& tcl = Tcl::instance();
		
		tcl.evalf("%s set node_", agent_->name());
		mobile_node_ = (MobileNode*) TclObject::lookup(tcl.result());

		Topography* T_;
		T_ = mobile_node_->get_topography();
		
		maxX = T_->upperX();
		maxY = T_->upperY();

		node_attribute();		// Specify the node attribute
		show_color();			// change the colour of the nodes
	}

	if (agent_response_)
	{
		set_up_target();		// Set response agent as an up-target

		if (node_attribute_ == BEACON)
			node_location_ = node_location(); // Get the acutual location of the node (only for beacon nodes)
	}
	if (agent_request_ && (node_attribute_ == UNKNOWN || node_attribute_ == REFERENCE)) 
	{
		requesting_ = 1;
		if (random_)
			start_time_ = Random::uniform(1.0, 5.0);

		// Specify which localisation algorithm should be run
		switch (method_)
		{
			case SINGLE:
				position_ = new Position(this);
				break;
			case NEAREST3:
				position_ = new NearestPosition(this);
				break;
			case REFINE:
				position_ = new RefinePosition(this);
				break;
		}
	}
}

// Specify the node attribute (beacon, referece or unknown)
void LocDisApp::node_attribute()
{
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s set node_", agent_->name());
	tcl.evalf("%s attribute",tcl.result());
	char attr[20];  
	strcpy(attr, tcl.result());

	if (strcmp(attr, "BEACON") == 0)
		node_attribute_ = BEACON;
	else if (strcmp(attr, "REFERENCE") == 0)
		node_attribute_ = REFERENCE;
	else if (strcmp(attr, "UNKNOWN") == 0)
		node_attribute_ = UNKNOWN;
	else
		tcl.eval("puts \"node attribute undefined!\"");
}

// if show_color is enabled then change the color of the node based on its attribute
void LocDisApp::show_color()
{
	if (show_color_ == 0)
		return;
	
	char *color = new char[20];	
	switch (node_attribute_)
	{
		case BEACON:
			color = "blue";
			break;
		case UNKNOWN:
			color = "yellow";
			break;
		case REFERENCE:
			color = "red";
			break;
		default:
			color = "black";
	}
	
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s set node_", agent_->name());
	tcl.evalf("%s color %s", tcl.result(), color);
}

// Get the acutual location of the node
Location LocDisApp::node_location()
{
	double x, y, z;
	Location loc;

	mobile_node_->getLoc(&x, &y, &z);
	
	loc.setLocation(x, y, z);
	return loc;
}

// Schedule the broadcasting of location request packet and the estimation time
void LocDisApp::timeout()
{
	if (!requesting_)
		return;

	if (final_)
	{
		requesting_ = 0;
		return;
	}

	data_.clear();
	agent_request_->broadcast();					// broadcast a location request packet
	double next_request_ = next_request();
	double next_estimate_  = next_request_ / 3.0;

	if (next_request_ > 0)
	{	
		req_timer_.resched(next_request_);		//schedule the next request
		est_timer_.resched(next_estimate_);		//schedule the next estimate
	}
	else
	{
		requesting_ = 0;
		est_timer_.resched(1.0);					//schedule the final estimate
	}
}

// Specify the time of the next request that will be used to broadcast a new location request packet
double LocDisApp::next_request()
{
	double nr = req_freq_;
	
	if (random_)
		nr += req_freq_ * Random::uniform(-0.5, 0.5);	

	if (++reqno_ < max_requests_)
		return(nr);
	else
		return(-1); 
}

// Extract the location information from the received location response packet and store it in a ResData vector
void LocDisApp::process_response(int size, Packet* pkt)
{
	hdr_ip* iph = HDR_IP(pkt);
	hdr_locres* lrh = HDR_LOCRES(pkt);

	nsaddr_t add = iph->src_.addr_;
	Location loc = lrh->node_location_;
	double	rxp = pkt->txinfo_.RxPr;

	ResData rdata(add, loc, rxp);

	add_response_data(&rdata);
}

// In this fuction the data is added without any order.
// However the data can be ranked based on a specific attribute such as rxp
void LocDisApp::add_response_data(ResData* rdata)
{
	data_.insert(data_.begin(), *rdata);
}

// Estimate the node location by calling the estimate function of a specific localisation algorithm that determined early (init())
void LocDisApp::location_estimate()
{
	if (data_.size() < 3)
		return;

	Location loc_;

	if(!position_->estimate(&loc_))
		return;

	node_location_ = loc_;
	node_attribute_ = REFERENCE;		//When unknown node estimate its position it becomes reference node
	loc_error_ = location_error();	//estimate the location error

	req_freq_ += 1.0;
	
	// if the unknown node get it's position then it becomes reference node then it's color should be changed
	if (!color_changed_)
	{
		show_color();
		color_changed_ = true;
	}
	// In this way the location info is logged directly and only once after the estimation.
	// to log the information every second the next line should be removed and the output timer should be resched every 1 sec
	output_timer_.resched(0.0);
}

// Estimate the location error
double LocDisApp::location_error()
{
	Location actual_location_;
	double error_;

	actual_location_ = node_location();

	error_ = actual_location_.distance(&node_location_);
	
	error_ = error_ * 100.0 / MAX_RANGE;
	
	return error_;
}

// Log the location information into the trace file
void LocDisApp::output()
{
	double x, y, z;
	
	// this can be used to log the information every 1 sec.
	//output_timer_.resched(1.0);

	if (loc_error_ < 0)
		return;

	//get the estimated location
	node_location_.getLocation(&x, &y, &z);
	//log node's location ifno
	mobile_node_->log_loc(loc_error_, no_ref_, x, y);
}	

// Set response agent as an up-target to handle the received packets from link layer
void LocDisApp::set_up_target()
{
	if(!agent_response_)
		return;

	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s set node_", agent_response_->name());
	tcl.evalf("[%s set ll_(0)] up-target %s",tcl.result(), agent_response_->name());
}

// this function will be invoked at the expiration of the request timer
void ReqTimer::expire(Event *)
{
	t_->timeout();
}
// this function will be invoked at the expiration of the estimate timer
void EstimateTimer::expire(Event *)
{
	t_->location_estimate();
}
// this function will be invoked at the expiration of the output timer
void OutputTimer::expire(Event *)
{
	t_->output();
}



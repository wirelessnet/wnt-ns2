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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/locationdiscovery.h
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: Header file.
 ************************************************************************************/

#ifndef ns_location_discovery_h
#define ns_location_discovery_h

#include <vector>
#include <string>
#include <fstream>
#include "tclcl.h"
#include "random.h"
#include "location.h"
#include "packet.h"
#include "agent.h"
#include "locationresponse.h"
#include "position.h"
#include "nearestposition.h"
#include "mobilenode.h"
#include "support.h"
#include "mmse.h"

class LocResAgent;
class LocReqAgent;
class LocDisApp;

// Timer for sending location request packets
class ReqTimer : public TimerHandler {
	public:
		ReqTimer( LocDisApp* t) : TimerHandler(), t_(t) {}
		inline virtual void expire(Event*);
	protected:
		 LocDisApp* t_;
};

// Timer for location estimation
class EstimateTimer : public TimerHandler {
	public:
		EstimateTimer( LocDisApp* t) : TimerHandler(), t_(t) {}
		inline virtual void expire(Event*);
	protected:
		 LocDisApp* t_;
};

// Timer for recording node's location information into the trace file
class OutputTimer : public TimerHandler {
	public:
		OutputTimer( LocDisApp* t) : TimerHandler(), t_(t) {}
		inline virtual void expire(Event*);
	protected:
		 LocDisApp* t_;
};

class LocDisApp : public Application {

	friend class Position;
	friend class NearestPosition;
	friend class RefinePosition;

	public:
		LocDisApp();
		~LocDisApp();
		virtual void timeout();
		virtual void location_estimate();
		virtual void output();
		virtual int command(int argc, const char*const* argv);
		virtual void process_response(int, Packet*);
		inline int get_node_att() { return node_attribute_;}							// Get the node attribute
		inline int get_method_type() { return method_;}									// Get the localisation method that should be used
		inline Location get_node_loc() { return node_location_;}						// Get the estimated location of the node
		inline void get_upper(double *x, double *y) { *x = maxX; *y = maxY;}		// Get the upper boundary of the topography
		inline void set_final(bool f) { final_ = f;}										// Get the value of the final_ flag
		inline void set_ref(int n) { no_ref_ = n;}										// Set the value of the number of references used in the estimation
		inline int get_dist_error() {return distance_error_;}							// Get the value of distance_error
		
	protected:
		virtual void start();
		virtual void init();
		virtual void node_attribute();
		virtual Location node_location();
		virtual double next_request();
		virtual void add_response_data(ResData*);
		virtual void show_color();
		virtual double location_error();
		void set_up_target();
		
		Position *position_;					// Pointer object of Position class
		LocReqAgent *agent_request_;		// Pointer object of LocReqAgent class
		LocResAgent *agent_response_;		// Pointer object of LocResAgent class
		MobileNode* mobile_node_;			// Pointer object of MobileNode class
		ReqTimer req_timer_;					// Request timer
		EstimateTimer est_timer_;			// Estimation timer
		OutputTimer output_timer_;			// output recording timer
		// A vector of ResData is used to store the location information that extracted from the received location response packets
		vector<ResData> data_;
		Location node_location_;			// The node's estimated location
		double req_freq_;						// Used to specify how frequent the node broadcast a location request packet
		double start_time_;					// Used to specify when to start broadcasting location request either directly or after a random time
													// based on the value of random_
		double loc_error_;					// the location error
		double maxX;							// is the x-coordination of the upper boundary of the topography
		double maxY;							// is the y-coordination of the upper boundary of the topography
		bool color_changed_;					// Flag used to indicate if the colour of the node is already changed
		bool final_;							// Flag used to indicate when to stop estimating the node location
		int node_attribute_;					// used to sepcify the node attribute; beacon, unknown, or reference
		int random_;							// specify the start time of broadcasting either directly or after a random time
		int distance_error_;					// used to determine if a distance measurement error will be added or not
		int max_requests_;					// the maximum number of location request packets that can be send
		int requesting_;						// Indicate if the node should continue broadcasting a location request packet or it should stop sending it
		int reqno_;								// specify the number of location request packets that have been sent
		int show_color_;						// determine if the colour of the node should be changed based on its attribute or not
		int method_;							// specify the localisation method that will be used in the estimation
		int no_ref_;							// number of references (or beacons) used in the estimation
};
#endif

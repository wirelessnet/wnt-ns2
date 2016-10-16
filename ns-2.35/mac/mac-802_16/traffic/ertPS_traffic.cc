/**************************************************************************************
* *Copyright (c) 2006 Regents of the University of Chang Gung 						*
* *All rights reserved.													*
 *																*
 * Redistribution and use in source and binary forms, with or without						*
 * modification, are permitted provided that the following conditions					*
 * are met: 															*
 * 1. Redistributions of source code must retain the above copyright						*
 *    notice, this list of conditions and the following disclaimer.						*
 * 2. Redistributions in binary form must reproduce the above copyright					*
 *    notice, this list of conditions and the following disclaimer in the						*
 *    documentation and/or other materials provided with the distribution.					*
 * 3. All advertising materials mentioning features or use of this software					*
 *    must display the following acknowledgement:									*
 *	This product includes software developed by the Computer Systems					*
 *	Engineering Group at Lawrence Berkeley Laboratory.							*
 * 4. Neither the name of the University nor of the Laboratory may be used					*
 *    to endorse or promote products derived from this software without					*
 *    specific prior written permission.										*
 *5. If you have any problem about these codes, 									*
       please mail to antibanish@gmail.com or b9229008@stmail.cgu.edu.tw                    			*
**************************************************************************************/
#include <stdlib.h>
 
#include "random.h"
#include "trafgen.h"
#include "ranvar.h"


/* 
 * Constant bit rate traffic source.   Parameterized by interval, (optional)
 * random noise in the interval, and packet size.  
 */
double talk_spurt = 1;
double silence_spurt = 0.1;
double duration = 0;
double last_duration = 0;
int talking_ = 1;

class ertPS_Traffic : public TrafficGenerator {
 public:
	ertPS_Traffic();
	virtual double next_interval(int&);
	//HACK so that udp agent knows interpacket arrival time within a burst
	inline double interval() { return (interval_); }
 protected:
	virtual void start();
	void init();
	double rate_;     /* send rate during on time (bps) */
	double interval_; /* packet inter-arrival time during burst (sec) */
	double random_;
	int seqno_;
	int maxpkts_;
};

static class ertPSTrafficClass : public TclClass {
 public:
	ertPSTrafficClass() : TclClass("Application/Traffic/ertPS") {}
	TclObject* create(int, const char*const*) {
		return (new ertPS_Traffic());
	}
} class_ertPS_Traffic;

ertPS_Traffic::ertPS_Traffic() : seqno_(0)
{
	rate_ = 64000;
	random_ = 0;
	size_ = 200;
	maxpkts_ = 268435456;
}

void ertPS_Traffic::init()
{
    // compute inter-packet interval 
	interval_ = (double)(size_ << 3)/(double)rate_;
	//printf("interval_ : %f \n",interval_);
	if (agent_)
		if (agent_->get_pkttype() != PT_TCP && agent_->get_pkttype() != PT_TFRC)
			//Setting packet type to ertPS.
			agent_->set_pkttype(PT_ertPS);
}

void ertPS_Traffic::start()
{
    init();
    running_ = 1;
    timeout();
}

double ertPS_Traffic::next_interval(int& size)
{		
	// Recompute interval in case rate_ or size_ has changes
	interval_ = (double)(200 << 3)/(double)rate_;
	double t = interval_;
	if (random_)
		t += interval_ * Random::uniform(-0.5, 0.5);
		
	if (talking_ == 1 && (duration - last_duration) > talk_spurt) {	
		//If talking time is large than talk-spurt.		
		size = 1;
		if (talking_ == 1) {
			last_duration = duration;
			talking_ = 0;
		}
		//printf("          ~~~~ change to silence ~~~~~\n");
	} else if (talking_ == 1 && (duration - last_duration) <= talk_spurt) {
		//If talking time is less than talk-spurt.
		size = 200;
		//printf("         ~~~~ talk mode ~~~~~\n");
	} else if (talking_ == 0 && (duration - last_duration) > silence_spurt) {
		//If silence time is large than silence-spurt.
		size = 200;
		if (talking_ == 0) {
			last_duration = duration;
			talking_ = 1;
		}
		//printf("          ~~~~ change to talk ~~~~~\n");
	} else if (talking_ == 0 && (duration - last_duration) <= silence_spurt) {
		//If silence time is less than silence-spurt.
		size = 1;
		//printf("          ~~~~ silence mode ~~~~~\n");
	}
	duration = duration + t;
	
	if (++seqno_ < maxpkts_)
		return(t);
	else
		return(-1); 
}


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

class rtPS_Traffic : public TrafficGenerator {
 public:
	rtPS_Traffic();
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

static class rtPSTrafficClass : public TclClass {
 public:
	rtPSTrafficClass() : TclClass("Application/Traffic/rtPS") {}
	TclObject* create(int, const char*const*) {
		return (new rtPS_Traffic());
	}
} class_rtPS_Traffic;

rtPS_Traffic::rtPS_Traffic() : seqno_(0)
{
	rate_ = 1024000;
	random_ = 1;
	size_ = 200;
	maxpkts_ = 268435456;
}

void rtPS_Traffic::init()
{
    // compute inter-packet interval 
	interval_ = (double)(size_ << 3)/(double)rate_;
	if (agent_)
		if (agent_->get_pkttype() != PT_TCP && agent_->get_pkttype() != PT_TFRC)
			//Setting packet type to rtPS.
			agent_->set_pkttype(PT_rtPS);
}

void rtPS_Traffic::start()
{
    init();
    running_ = 1;
    timeout();
}

double rtPS_Traffic::next_interval(int& size)
{
	// Recompute interval in case rate_ or size_ has changes
	interval_ = (double)(size << 3)/(double)rate_;
	double t = interval_;
	if (random_)
		t += interval_ * Random::uniform(-0.5, 0.5);
	//nrtPS service using variable bit rate packet.
	size = (int)Random::uniform(200, 1000);
	
	if (++seqno_ < maxpkts_)
		return(t);
	else
		return(-1); 
}


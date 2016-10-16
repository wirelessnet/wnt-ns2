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
#include "priority.h"


/* 
 * Constant bit rate traffic source.   Parameterized by interval, (optional)
 * random noise in the interval, and packet size.  
 */

class UGS_Traffic : public TrafficGenerator {
 public:
	UGS_Traffic();
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

static class UGSTrafficClass : public TclClass {
 public:
	UGSTrafficClass() : TclClass("Application/Traffic/UGS") {}
	TclObject* create(int, const char*const*) {
		return (new UGS_Traffic());
	}
} class_UGS_Traffic;

UGS_Traffic::UGS_Traffic() : seqno_(0)
{
	rate_ = 64000;
	random_ = 0;
	size_ = 200;
	maxpkts_ = 268435456;
}

void UGS_Traffic::init()
{
    // compute inter-packet interval 
	interval_ = (double)(size_ << 3)/(double)rate_;
	if (agent_)
		if (agent_->get_pkttype() != PT_TCP && agent_->get_pkttype() != PT_TFRC)
			//Setting packet type to UGS.
			agent_->set_pkttype(PT_UGS);
			//x.setpriority(agent_,1);
}

void UGS_Traffic::start()
{
    init();
    running_ = 1;
    timeout();
}

double UGS_Traffic::next_interval(int& size)
{
	// Recompute interval in case rate_ or size_ has changes
	interval_ = (double)(size << 3)/(double)rate_;
	double t = interval_;
	if (random_)
		t += interval_ * Random::uniform(-0.5, 0.5);
	//UGS service using constant bit rate packet.	
	size = 200;
	
	if (++seqno_ < maxpkts_)
		return(t);
	else
		return(-1); 
}


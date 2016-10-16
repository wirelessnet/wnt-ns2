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

#include "random.h"
#include "tcp.h"
#include "nrtPS_traffic.h"

extern double tcplib_telnet_interarrival();

static class nrtPSAppClass : public TclClass {
 public:
	nrtPSAppClass() : TclClass("Application/nrtPS") {}
	TclObject* create(int, const char*const*) {
		return (new nrtPSApp);
	}
} class_app_nrtPS;


nrtPSApp::nrtPSApp() : running_(0), timer_(this)
{
	//bind("interval_", &interval_);
	interval_ = 0.01;
}


void nrtPSAppTimer::expire(Event*)
{
    t_->timeout();
}


void nrtPSApp::start()
{
    running_ = 1;
	double t = next();
	timer_.sched(t);
}

void nrtPSApp::stop()
{
    running_ = 0;
}

void nrtPSApp::timeout()
{
    if (running_) {
		//nrtPS service using variable bit rate packet.
		size_ = (int)Random::uniform(200, 1000);
		agent_->size() = size_;
		agent_->send(agent_->size());
		//Setting packet type to nrtPS.
		agent_->set_pkttype(PT_nrtPS);
		/* reschedule the timer */
		double t = next();
		timer_.resched(t);
	}
}

double nrtPSApp::next()
{
	return interval_;
}

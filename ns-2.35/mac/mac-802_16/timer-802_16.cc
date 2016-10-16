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
#include "timer-802_16.h"
#include <packet.h>
#include <assert.h>
#include <random.h>
#include <scheduler.h>
#include "mac-802_16.h"

void Mac802_16Timer::start(double time)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	wtime = time;
	assert(wtime >= 0.0);
	////event.uid_ = 0;
	s.schedule(this, &event, wtime);
}
 
void Mac802_16Timer::stop(void)
{ 
	Scheduler &s = Scheduler::instance();

	assert(busy_);
	if(paused_ == 0)
		s.cancel(&event);
}

void Mac802_16Timer::handle(Event* e){}

void FrameTimer::start(double time)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	rtime = time;
	intr.uid_=0;
	assert(rtime >= 0.0);
	s.schedule(this, &intr, rtime);
}

void FrameTimer::handle(Event* e)
{ 
	busy_=0;
	paused_=0;
	stime=0.0;
	wtime=0.0;
	mac->FrameTimerHandler();
}

void PreambleTimer::start(double time)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	rtime = time;
	intr.uid_=0;
	assert(rtime >= 0.0);
	s.schedule(this, &intr, rtime);
}

void PreambleTimer::handle(Event* e)
{ 
	busy_=0;
	paused_=0;
	stime=0.0;
	wtime=0.0;
	mac->PreambleTimerHandler();
}

void DownlinkTimer::start(double time, Packet *p)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	pkt = p;
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	rtime = time;
	intr.uid_=0;
	assert(rtime >= 0.0);
	s.schedule(this, &intr, rtime);
}

void DownlinkTimer::handle(Event* e)
{ 
	busy_=0;
	paused_=0;
	stime=0.0;
	wtime=0.0;
	mac->DownlinkTimerHandler(pkt);
}

void UplinkTimer::start(double time, Packet *p)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	pkt = p;
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	rtime = time;
	intr.uid_=0;
	assert(rtime >= 0.0);
	s.schedule(this, &intr, rtime);
}

void UplinkTimer::handle(Event* e)
{ 
	busy_=0;
	paused_=0;
	stime=0.0;
	wtime=0.0;
	mac->UplinkTimerHandler(pkt);
}

void UlMapTimer::start(double time)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	rtime = time;
	intr.uid_=0;
	assert(rtime >= 0.0);
	s.schedule(this, &intr, rtime);
}

void UlMapTimer::handle(Event* e)
{ 
	busy_=0;
	paused_=0;
	stime=0.0;
	wtime=0.0;
	mac->SsScheduler();
}

void DlMapTimer::start(double time)
{
	Scheduler &s = Scheduler::instance();
	assert(busy_ == 0);
	busy_ = 1;
	paused_ = 0;
	stime = s.clock();
	rtime = time;
	intr.uid_=0;
	assert(rtime >= 0.0);
	s.schedule(this, &intr, rtime);
}

void DlMapTimer::handle(Event* e)
{ 
	busy_=0;
	paused_=0;
	stime=0.0;
	wtime=0.0;
	mac->BsScheduler();
}

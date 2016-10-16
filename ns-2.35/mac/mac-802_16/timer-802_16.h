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
#ifndef __mac_timers_80216_h__
#define __mac_timers_80216_h__

#include <scheduler.h>
#include <assert.h>
#include <packet.h>
#include "packet-802_16.h"

class Mac802_16;

class Mac802_16Timer : public Handler
{
	public:
		Mac802_16Timer(Mac802_16* m):mac(m),rtime(0){busy_ = paused_ = 0; stime = 0.0;};
		virtual void	handle(Event *e)=0;		
		virtual void	start(double time);
		virtual void	stop(void);
		virtual void	pause(void) {assert(0);}
		virtual void	resume(void) {assert(0);}
		inline int	busy(void) {return busy_;}
		inline int	paused(void) {return paused_;}

	protected:
		Mac802_16	*mac;
		int			busy_;
		int			paused_;
		Event		event;
		Event		intr;		
		double		stime;		//start time
		double		wtime;		//waiting time
		double      rtime;
};

class FrameTimer:public Mac802_16Timer{
	public:
		FrameTimer(Mac802_16* m):Mac802_16Timer(m){};
		void 	start(double time);		
		void	handle(Event *e);
};

class PreambleTimer:public Mac802_16Timer{
	public:
		PreambleTimer(Mac802_16* m):Mac802_16Timer(m){};
		void 	start(double time);		
		void	handle(Event *e);
};

class DownlinkTimer:public Mac802_16Timer{
	public:
		DownlinkTimer(Mac802_16* m):Mac802_16Timer(m){};
		void 	start(double time, Packet *p);		
		void	handle(Event *e);
	protected:
		Packet *pkt;
};

class UplinkTimer:public Mac802_16Timer{
	public:
		UplinkTimer(Mac802_16* m):Mac802_16Timer(m){};
		void 	start(double time, Packet *p);		
		void	handle(Event *e);
	protected:
		Packet *pkt;
};

class UlMapTimer:public Mac802_16Timer{
	public:
		UlMapTimer(Mac802_16* m):Mac802_16Timer(m){};
		void 	start(double time);		
		void	handle(Event *e);
};

class DlMapTimer:public Mac802_16Timer{
	public:
		DlMapTimer(Mac802_16* m):Mac802_16Timer(m){};
		void 	start(double time);		
		void	handle(Event *e);
};
#endif

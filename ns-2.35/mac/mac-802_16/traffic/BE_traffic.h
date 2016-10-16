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

#ifndef ns_BE_h
#define ns_BE_h

#include "timer-handler.h"
#include "app.h"

class TcpAgent;
class TfrcAgent;
class BEApp;

class BEAppTimer : public TimerHandler {
 public:
	BEAppTimer(BEApp* t) : TimerHandler(), t_(t) {}
	inline virtual void expire(Event*);
 protected:
	BEApp* t_;
};


class BEApp : public Application {
 public:
	BEApp();
	void timeout();
 protected:
	void start();
	void stop();
	inline double next();

	double interval_;
	int size_;
	int running_;
	BEAppTimer timer_;
};

#endif

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
 *		Description: Header file
 ************************************************************************************/

#ifndef ns_location_response_h
#define ns_location_response_h

#include "agent.h"
#include "ip.h"
#include "locationdiscovery.h"
#include "locationrequest.h"

class LocDisApp;

class LocResAgent : public Agent {
	
	friend class LocDisApp;
	
	public:
		LocResAgent();
		LocResAgent(packet_t type);
		~LocResAgent();

		virtual void attachApp(LocDisApp* app);

	protected:
		virtual void recv(Packet*, Handler*);
		virtual void send_res(Packet*, Handler*);
		virtual void recv_res(Packet*, Handler*);
		inline NsObject* get_link_layer();

		LocDisApp* ldapp_;
		bool set_target_;		// this flag is used to indicate that the link layer has been already setted as target
};
#endif

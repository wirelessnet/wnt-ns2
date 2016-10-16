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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/locationpacket.h	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: In this file two protocol specific header C++ structures have
 *						 been defined. “hdr_locreq” structure for the location request
 *						 packet (PT_LOCREQ) and “hdr_locres” structure for the location
 *						 response packet (PT_LOCRES).
 ************************************************************************************/


#ifndef ns_location_packet_h
#define ns_location_packet_h

#define HDR_LOCREQ(p)      (hdr_locreq::access(p))		// location request
#define HDR_LOCRES(p)		(hdr_locres::access(p))		// location response
#define LOCRES_PORT			532								// same port reserved for netnews

class Location;

//Location request header structure
struct hdr_locreq
{
	static int offset_;

	inline static int& offset() {return offset_; }
	inline static hdr_locreq* access(const Packet* p) {
		return (hdr_locreq*) p->access(offset_);
  }
};

//Location response header structure
struct hdr_locres
{
	Location node_location_;
  	
	static int offset_;
	inline static int& offset() {return offset_; }
	inline static hdr_locres* access(const Packet* p) {
		return (hdr_locres*) p->access(offset_);
  }
	Location& loc() { return (node_location_);}
};		
#endif

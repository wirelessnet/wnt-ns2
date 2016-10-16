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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/support.h
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: Header file which defines ResData and other common parameters
 ************************************************************************************/

#ifndef ns_support_h
#define ns_support_h

#include "location.h"
#include "packet.h"

#define SUCCESS			true
#define FAIL				false

//node attribute:
#define BEACON				1
#define REFERENCE			2
#define UNKNOWN			3

// location estimation method:
#define SINGLE				1					//Which implemented in position.h,.cc
#define NEAREST3			2					//Which implemented in nearestposition.h,.cc
#define REFINE				3					//Which implemented in refineposition.h,.cc

#define SPEED_OF_LIGHT	300000000			// 3 * 10^8 m/s
#define PI					3.1415926535897

// from the terminal cd to ~/ns-allinone-2.34/ns-2.34/indep-utils/propagation$ then run:
// ./threshold -m FreeSpace -r 0.95 50

//this mean that the propagation-model is FreeSpace and 95% packets is correctly received at the distance of 50m.
//then you will get the configuration parameters as following:
//distance = 50
//propagation model: FreeSpace

//Selected parameters:
//transmit power: 0.281838
//frequency: 9.14e+08
//transmit antenna gain: 1
//receive antenna gain: 1
//system loss: 1

//Which can be used to set following:
#define TX_POWER			0.281838									// transmit power
#define TX_GAIN			1.0										// transmitter gain
#define RX_GAIN			1.0										// receiver gain
#define SYS_LOSS			1.0										// system loss (L >= 1)
#define FREQ				914000000.0								// 9.14e+08
#define LAMBDA				(SPEED_OF_LIGHT / FREQ)				// 0.328227571
#define MAX_RANGE			50.0										// max transmission range

inline double square(double t) {return t * t;}

// This class is used to capture the data that is required by the localisation system
// Generally this data include the address and location of the reference nodes that send the location response packets, 
// and the power at which these packet are received
// Implementing new algorithms could require adding extra attributes to this class
class ResData : public TclObject
{
	public:
		ResData() : src_add_(0), src_loc_(), pkt_rxp_(0) {}
		
		ResData(nsaddr_t sd, Location sl, double rp) : src_add_(sd), src_loc_(sl), pkt_rxp_(rp) {}

		inline double getrxp()		{return pkt_rxp_;}
		inline Location getloc()	{return src_loc_;}
		inline nsaddr_t getadd()	{return src_add_;}

		inline void setResData(nsaddr_t sd, Location sl, double rp)
		{
			src_add_ = sd;
			src_loc_ = sl;
			pkt_rxp_ = rp;
		}
		inline void getResData(nsaddr_t *sd, Location *sl, double *rp)
		{
			*sd = src_add_;
			*sl = src_loc_;
			*rp = pkt_rxp_;
		}
		virtual bool is_equal(ResData* rd)
		{
			if ((src_add_ == rd->src_add_) && 
					(src_loc_.is_equal(&rd->src_loc_)) &&
					(pkt_rxp_ == rd->pkt_rxp_))
				return true;
			else
				return false;
		}

	protected:
		nsaddr_t src_add_;		// source ip address
		Location	src_loc_;		// source location
		double	pkt_rxp_;		// power with which pkt is received
};

// This structure consists of two members, which represent  the location of and the distance to the reference node
struct ReferenceNode
{
	Location loc_;
	double distance_;
};
#endif

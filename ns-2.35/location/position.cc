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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/position.cc	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: Position class represents the general multilateration method [1]
 *						 to estimate the node position. This method uses all of the
 * 					 available references, does not distinguish between beacons and
 *						 references, does not weigh the references used and performs
 *						 the estimation only once. This class is the base class for any
 *						 localization algorithms that will be implemented.
 *
 *					[1] A. Savvides, C. C. Han and M. B. Strivastava, "Dynamic 
 *						 fine-grained localization in ad-hoc networks of sensors," in 
 *						 Proceedings of the 7th Annual International Conference on Mobile
 *						 Computing and Networking — MobiCom '01, July 16-21, Rome, Italy,
 *						 2001, pp. 166-179.
 ************************************************************************************/

#include "locationdiscovery.h"

// Constructor
Position::Position(LocDisApp* l) : ldis_(l), num_ref_(0) {}

// Distructor
Position::~Position()
{
	delete [] ref_nodes_;
	delete ldis_;
}

// Estimate node position
bool Position::estimate(Location* loc_)
{
	init();

	MMSE mmse;

	if(!mmse.estimate(ref_nodes_, num_ref_, loc_))
		return FAIL;
	
	boundary(loc_);

	ldis_->set_ref(num_ref_);
	
	ldis_->set_final(true);
	
	return SUCCESS;
}

bool Position::init()
{
	num_ref_ = ldis_->data_.size();
	ref_nodes_ = new ReferenceNode[num_ref_];
	set_ref_nodes();
	return SUCCESS;
}

// This function extract the required information to apply the MMSE technique from the data_ vector, which hold the received information
// from the neighbour references. This information will be stored in ref_nodes_
void Position::set_ref_nodes()
{
	double rxp;
	int i;

	for (i = 0; i < num_ref_; i++)
	{
		rxp = ldis_->data_.at(i).getrxp();
		ref_nodes_[i].loc_ =  ldis_->data_.at(i).getloc();
		ref_nodes_[i].distance_ = measure_distance_RSS(rxp);
	}
}

// Using Friis free space equation to estimate the measure distance between the node and the reference (or beacon) node
double Position::measure_distance_RSS(double Pr)
{
	double Pt = TX_POWER;
	double Gt = TX_GAIN;
	double Gr = RX_GAIN;
	double lambda = LAMBDA;
	double L = SYS_LOSS;
	double distance;
	RNG error;
	double mean, sd;

	distance = sqrt((Pt * Gt * Gr) / (L * Pr)) * lambda / (4 * PI);

	if (ldis_->get_dist_error())
	{
		// Add error
		// Increasing the standard deviation (sd) will increase the level of error
		mean = 0.001 * distance;
		sd = 0.01 * distance;
	
		distance += error.normal(mean, sd);
	}

	return distance;	
}

// This function is used to make sure that the estimated position is within the boundary of the topography
void Position::boundary(Location * loc_)
{	
	double maxX, maxY;
	ldis_->get_upper(&maxX, &maxY);

	if (loc_->getx() < 0)
		loc_->setx(0);
	if (loc_->getx() > maxX)
		loc_->setx(maxX);
	if (loc_->gety() < 0)
		loc_->sety(0);
	if (loc_->gety() > maxY)
		loc_->sety(maxY);
}

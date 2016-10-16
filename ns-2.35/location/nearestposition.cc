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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/nearestposition.cc	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: NearestPosition class is derived from Position class. This class
 *						 represents the localisation algorithm presented in [2]. The node
 *						 selects a subset of three references to estimate its position.
 *						 The selected references have the minimum measured distance to the
 *						 node (i.e. the closest three references to the node).
 *
 *					[2] K. Y. Cheng, V. Tam and K. S. Lui, "Improving aps with anchor
 *						 selection in anisotropic sensor networks," in Proceedings of the
 *						 Joint International Conference on Autonomic and Autonomous Systems
 *						 and International Conference on Networking and Services
 *						 — ICAS-ICNS '05, October 23-28, Papeete, Tahiti, 2005, pp. 49-54.
 ************************************************************************************/

#include "locationdiscovery.h"

// Constructor
NearestPosition::NearestPosition(LocDisApp* l) :  Position(l) {}

// Estimate node position
bool NearestPosition::estimate(Location* loc_)
{
	init();

	num_ref_ = 3;
	nearest_ref();

	MMSE mmse;

	if(!mmse.estimate(ref_nodes_, num_ref_, loc_))
		return FAIL;
	
	boundary(loc_);

	ldis_->set_ref(num_ref_);
	
	ldis_->set_final(true);
	
	return SUCCESS;
}

// Find the closest three references to the node
void NearestPosition::nearest_ref()
{
	unsigned int i, j, index;
	double min;
	ReferenceNode Ref;

	for (j = 0; j < num_ref_; j++)
	{
		min = ref_nodes_[j].distance_;
		index = 0;
		for (i = j + 1; i < ldis_->data_.size(); i++)
		{			
			if (ref_nodes_[i].distance_ < min)
			{
				min = ref_nodes_[i].distance_;
				index = i;
			}
		}
		if (index)
		{
			Ref = ref_nodes_[index];
			ref_nodes_[index] =  ref_nodes_[j];
			ref_nodes_[j] = Ref;
		}
	}
}


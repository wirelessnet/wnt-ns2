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
 *		File: ~/ns-allinone-2.34/ns-2.34/location/mmse.cc	
 *
 *		Author: Adnan Abu-Mahfouz
 *
 *		Date: March 2012
 *
 *		Description: This class is responsible for all the mathematical matrices
 *						 operations required to obtain the minimum mean square estimate
 *						 (MMSE). Instead of using a general matrices multiplication and
 *						 matrix inverse, optimised methods dedicated mainly to MMSE were
 *						 implemented. These optimised methods require less computation
 *						 and shorter execution time.
 ************************************************************************************/

#include "mmse.h"

// Distructor
MMSE::~MMSE()
{
	delete [] X;
	delete [] Y;
}

// Solve the equation: b = Inverse(XT * X) * XT * Y
// b is the estimated location (loc)
bool MMSE::estimate(ReferenceNode *ref_nodes_, int n, Location* loc_)
{
	double Z[2][2] = {0.0};
	double W[2] = {0.0};

	num_ref_ = n;

	init(ref_nodes_);

	multiply_XT_X(Z);

	if (!inverse_XTX(Z))
		return FAIL;

	multiply_XT_Y(W);

	// multiply Z by W to get the location
	double x, y;

	x = Z[0][0] * W[0] + Z[0][1] * W[1];
	y = Z[1][0] * W[0] + Z[1][1] * W[1];
	
	loc_->setx(x);
	loc_->sety(y);

	return SUCCESS;
}

/***************************************************************
* create the matrices X and Y:
*	where m is the number of reference nodes minuse 1
*
*			|	2(x0 - x1)			2(y0 - y1)	|
*			|	2(x0 - x2)			2(y0 - y2)	|
*			|	2(x0 - x3)			2(y0 - y3)	|
*	X	= 	|		.						.			|
*			|		.						.			|
*			|		.						.			|
*			|	2(x0 - xm)			2(y0 - ym)	|
*
*
*
*			|	x0^2 + y0^2 - d0^2 - x1^2 - y1^2 + d1^2	|
*			|	x0^2 + y0^2 - d0^2 - x2^2 - y2^2 + d2^2	|
*			|	x0^2 + y0^2 - d0^2 - x3^2 - y3^2 + d3^2	|
*	Y	=	|							.								|
*			|							.								|
*			|							.								|
*			|	x0^2 + y0^2 - d0^2 - xm^2 - ym^2 + dm^2	|
*
*
****************************************************************/

void MMSE::init(ReferenceNode *ref_nodes_)
{
	double x0, y0, d0;
	double x, y, d, t;
	int i;

	Y = new double [num_ref_ -1];

	X = new double *[num_ref_ -1];
	
	for (i = 0; i < num_ref_ - 1; i++)
		X[i] = new double[2];

	x0 = ref_nodes_[0].loc_.getx();
	y0 = ref_nodes_[0].loc_.gety();
	d0 = ref_nodes_[0].distance_;

	t = square(x0) + square(y0) - square(d0);

	for (i = 0; i < num_ref_ - 1; i++)
	{
		x = ref_nodes_[i + 1].loc_.getx();
		y = ref_nodes_[i + 1].loc_.gety();
		d = ref_nodes_[i + 1].distance_;
		
		X[i][0] = 2 * (x0 - x);
		X[i][1] = 2 * (y0 - y);

		Y[i] = t - square(x) - square(y) + square(d);
	}
}
// calculate the multiplication of XT by X (Z = XT * X) the result matrix (Z) is always 2 x 2 matrix 
void MMSE::multiply_XT_X(double Z[][2])
{
	int i;

	for (i = 0; i < num_ref_ - 1; i++)
	{
		Z[0][0] += square(X[i][0]);
		Z[0][1] += X[i][0] * X[i][1];
		Z[1][1] += square(X[i][1]);
	}
	Z[1][0] = Z[0][1];
}

// Z is the inverse of (XT * X)
bool MMSE::inverse_XTX(double Z[][2])
{
	double determinant_;

	determinant_ = Z[0][0] * Z[1][1] - Z[0][1] * Z[1][0];

	if (determinant_ == 0)
		return FAIL;

	double t = Z[0][0];

	Z[0][0] = Z[1][1] / determinant_;
	Z[0][1] *= -1 / determinant_;
	Z[1][0] *= -1 / determinant_;
	Z[1][1] = t / determinant_;

	return SUCCESS;
}

// W is the multiplication of XT by Y (W = XT * Y) W is always 2 x 1 matirx
void MMSE::multiply_XT_Y(double W[])
{
	int i;

	for (i = 0; i < num_ref_ - 1; i++)
	{
		W[0] += X[i][0] * Y[i];
		W[1] += X[i][1] * Y[i];
	}
}

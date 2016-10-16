// File:    zbr_link.h
// Mode:    C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t
// Author:  Jianliang Zheng

/*
 * Copyright (c) 2003-2004 Samsung Advanced Institute of Technology and
 * The City University of New York. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Joint Lab of Samsung 
 *      Advanced Institute of Technology and The City University of New York.
 * 4. Neither the name of Samsung Advanced Institute of Technology nor of 
 *    The City University of New York may be used to endorse or promote 
 *    products derived from this software without specific prior written 
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE JOINT LAB OF SAMSUNG ADVANCED INSTITUTE
 * OF TECHNOLOGY AND THE CITY UNIVERSITY OF NEW YORK ``AS IS'' AND ANY EXPRESS 
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN 
 * NO EVENT SHALL SAMSUNG ADVANCED INSTITUTE OR THE CITY UNIVERSITY OF NEW YORK 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef zbr_link_h
#define zbr_link_h

#include <stdlib.h>

#define zbr_oper_del	1
#define zbr_oper_est	2


class ZBR;

class ZBRLINK 
{
public:
	int addr;
	ZBR *zbr;
	ZBRLINK *last;
	ZBRLINK *next;
	ZBRLINK(int ad,ZBR *z)
	{
		addr = ad;
		zbr = z;
		last = NULL;
		next = NULL;
	}
};

int addZBRLink(int ad,ZBR *z);
int updateZBRLink(int oper,int ad);
ZBR *getZBRLink(int ad);
int chkAddZBRLink(int ad,ZBR *z);

//--------------------------------------------------------

class CLUSTADDRLINK 
{
public:
	char flag;
	int ipAddr;
	unsigned int logAddr;
	CLUSTADDRLINK *last;
	CLUSTADDRLINK *next;
	CLUSTADDRLINK(unsigned int logad,int ipad)
	{
		flag = 'n';
		ipAddr = ipad;
		logAddr = logad;
		last = NULL;
		next = NULL;
	}
};

int addCTAddrLink(unsigned int logad,int ipad);
int updateCTAddrLink(int oper,unsigned int logad);
void delObsoCTAddrLink(unsigned int logad,int ipad);
void activateCTAddrLink(unsigned int logad,int ipad);
int getIpAddrFrLogAddr(unsigned int logad,bool activeOnly = true);
unsigned int getLogAddrFrIpAddr(int ipad,bool activeOnly = true);
int chkAddCTAddrLink(unsigned int logad,int ipad);

#endif

// End of file: zbr_link.h

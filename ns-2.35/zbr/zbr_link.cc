// File:    zbr_link.cc
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


#include "zbr_link.h"


ZBRLINK *zbrLink1 = NULL;
ZBRLINK *zbrLink2 = NULL;

int addZBRLink(int ad,ZBR *z)
{
	///add an element into the link (not check duplication)
	///return values:
	///	0 = successful
	///	1 = failed
	ZBRLINK *tmp;

	if (zbrLink2 == NULL)		//not exist yet
	{
		zbrLink2 = new ZBRLINK(ad,z);
		if (zbrLink2 == NULL) return 1;
		zbrLink1 = zbrLink2;
	}
	else
	{
		tmp=new ZBRLINK(ad,z);
		if (tmp == NULL) return 1;
		tmp->last = zbrLink2;
		zbrLink2->next = tmp;
		zbrLink2 = tmp;
	}
	return 0;
}

int updateZBRLink(int oper,int ad)
{
	///return values:
	///	0 = successful
	///	1 = failed (not exist)
	///if (oper == zbr_oper_del) then (delete the corresponding element)
	///if (oper == zbr_oper_est) then (test whether the corresponding element exists, nothing changed)
	ZBRLINK *tmp;
	int rt;

	rt = 1;

	tmp = zbrLink1;
	while(tmp != NULL)
	{
		if (tmp->addr == ad)
		{
			if (oper == zbr_oper_del)	//delete an element
			{
				if(tmp->last != NULL)
				{
					tmp->last->next = tmp->next;
					if(tmp->next != NULL)
						tmp->next->last = tmp->last;
					else
						zbrLink2 = tmp->last;
				}
				else if (tmp->next != NULL)
				{
					zbrLink1 = tmp->next;
					tmp->next->last = NULL;
				}
				else
				{
					zbrLink1 = NULL;
					zbrLink2 = NULL;
				}
				delete tmp;
			}
			rt = 0;
			break;
		}
		tmp = tmp->next;
	}
	return rt;
}

ZBR *getZBRLink(int ad)
{
	///return values:
	///	ZBR instance if found
	///	NULL otherwise
	ZBRLINK *tmp;

	tmp = zbrLink1;
	while(tmp != NULL)
	{
		if(tmp->addr == ad)
			return tmp->zbr;
		tmp = tmp->next;
	}

	return NULL;
}

int chkAddZBRLink(int ad,ZBR *z)
{
        ///add an element into the link (check duplications first)
        ///return values:
        ///      0 = successful
        ///      1 = the element already exists
        ///      2 = failed
        int i;

        i = updateZBRLink(zbr_oper_est,ad);
        if (i == 0) return 1;
        i = addZBRLink(ad,z);
        if (i == 0) return 0;
        else return 2;
}

//-------------------------------------------------------------------

CLUSTADDRLINK *ctAddrLink1 = NULL;
CLUSTADDRLINK *ctAddrLink2 = NULL;

int addCTAddrLink(unsigned int logad,int ipad)
{
	///add an element into the link (not check duplication)
	///return values:
	///	0 = successful
	///	1 = failed
	CLUSTADDRLINK *tmp;

	if (ctAddrLink2 == NULL)		//not exist yet
	{
		ctAddrLink2 = new CLUSTADDRLINK(logad,ipad);
		if (ctAddrLink2 == NULL) return 1;
		ctAddrLink1 = ctAddrLink2;
	}
	else
	{
		tmp=new CLUSTADDRLINK(logad,ipad);
		if (tmp == NULL) return 1;
		tmp->last = ctAddrLink2;
		ctAddrLink2->next = tmp;
		ctAddrLink2 = tmp;
	}
	return 0;
}

int updateCTAddrLink(int oper,unsigned int logad)
{
	///return values:
	///	0 = successful
	///	1 = failed (not exist)
	///if (oper == zbr_oper_del) then (delete the corresponding element)
	///if (oper == zbr_oper_est) then (test whether the corresponding element exists, nothing changed)
	CLUSTADDRLINK *tmp;
	int rt;

	rt = 1;

	tmp = ctAddrLink1;
	while(tmp != NULL)
	{
		if (tmp->logAddr == logad)
		{
			if (oper == zbr_oper_del)	//delete an element
			{
				if(tmp->last != NULL)
				{
					tmp->last->next = tmp->next;
					if(tmp->next != NULL)
						tmp->next->last = tmp->last;
					else
						ctAddrLink2 = tmp->last;
				}
				else if (tmp->next != NULL)
				{
					ctAddrLink1 = tmp->next;
					tmp->next->last = NULL;
				}
				else
				{
					ctAddrLink1 = NULL;
					ctAddrLink2 = NULL;
				}
				delete tmp;
			}
			rt = 0;
			break;
		}
		tmp = tmp->next;
	}
	return rt;
}

void delObsoCTAddrLink(unsigned int logad,int ipad)
{
	CLUSTADDRLINK *tmp;

	tmp = ctAddrLink1;
	while(tmp != NULL)
	{
		if (((tmp->logAddr == logad)&&(tmp->ipAddr != ipad))
		|| ((tmp->logAddr != logad)&&(tmp->ipAddr == ipad)))
		{
			updateCTAddrLink(zbr_oper_del,tmp->logAddr);
			break;
		}
		tmp = tmp->next;
	}
}

void activateCTAddrLink(unsigned int logad,int ipad)
{
	CLUSTADDRLINK *tmp;

	tmp = ctAddrLink1;
	while(tmp != NULL)
	{
		if ((tmp->logAddr == logad)&&(tmp->ipAddr == ipad))
		{
			tmp->flag = 'y';
			break;
		}
		tmp = tmp->next;
	}
}

int getIpAddrFrLogAddr(unsigned int logad,bool activeOnly)
{
	///return values:
	///	the ipAddr if found
	///	-1 if not found (or not yet activated if <activeOnly> == true)
	CLUSTADDRLINK *tmp;

	tmp = ctAddrLink1;
	while(tmp != NULL)
	{
		if (tmp->logAddr == logad)
		{
			if (!activeOnly)
				return tmp->ipAddr;
			else if (tmp->flag == 'y')
				return tmp->ipAddr;
			else
				return -1;
		}
		tmp = tmp->next;
	}

	return -1;
}

unsigned int getLogAddrFrIpAddr(int ipad,bool activeOnly)
{
	///return values:
	///	the ipAddr if found
	///	0xffff if not found (or not yet activated if <activeOnly> == true)
	CLUSTADDRLINK *tmp;

	tmp = ctAddrLink1;
	while(tmp != NULL)
	{
		if (tmp->ipAddr == ipad)
		{
			if (!activeOnly)
				return tmp->logAddr;
			else if (tmp->flag == 'y')
				return tmp->logAddr;
			else
				return 0xffff;
		}
		tmp = tmp->next;
	}

	return 0xffff;
}

int chkAddCTAddrLink(unsigned int logad,int ipad)
{
        ///add an element into the link (check duplications first)
        ///return values:
        ///      0 = successful
        ///      1 = the element already exists
        ///      2 = failed
        int i;

	delObsoCTAddrLink(logad,ipad);
        i = updateCTAddrLink(zbr_oper_est,logad);
        if (i == 0) return 1;
        i = addCTAddrLink(logad,ipad);
        if (i == 0) return 0;
        else return 2;
}

// End of file: p802_15_4nam.cc

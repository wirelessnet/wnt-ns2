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
#include "delay.h"
#include "connector.h"
#include "packet.h"
#include "random.h"
#include "mobilenode.h"

#include "arp.h" 
#include "ll.h"
#include "mac.h"
#include "mac-802_16.h"
#include "cmu-trace.h"
#include "rtp.h"   
#include "agent.h"
#include "basetrace.h"
#include "geographic.h"

#define BsID 0
#define MaxProfileNum 6
#define PDUSIZE 100
#define DlSymbolNum 36
#define UlSymbolNum 12
#define DlSubchanNum 60
#define UlSubchanNum 92

extern TOMAC *CalMod;

int BasicCID_Counter = 0x0001; //0x0001 ~ 0x1000
int PrimaryCID_Counter = 0x001; //0x1001 ~ 0x2000
int TransportCID_Counter = 0x2001; //0x2001 ~ 0xFFFE
int FrameNumber_Counter = 0;
int DCD_Counter = 0;
int UCD_Counter = 0;
int ServiceFlowID_Counter0 = 0;
int ServiceFlowID_Counter1 = 0;
int ServiceFlowID_Counter2 = 0;
int ServiceFlowID_Counter3 = 0;
int ServiceFlowID_Counter4 = 0;
int ServiceFlowID_Counter5 = 0;
int ServiceFlowID_Counter6 = 0;
int ServiceFlowID_Counter7 = 0;
int ServiceFlowID_Counter8 = 0;
int ServiceFlowID_Counter9 = 0;
int UlMapIeNum, DlMapIeNum;
int DlSymbolMap[DlSymbolNum][DlSubchanNum];
int UlSymbolMap[UlSymbolNum][UlSubchanNum];
double FrameLength = 0.005, SymbolLength = 0.00010084, TtgLength = 0.00002941, RtgLength = 0.00002941;
double AllocationStartTime;
int DlAvailSymbol, UlAvailSymbol;
BsSsInfo *BsSsInfo_head;
BsSsInfo *BsSsInfo_tail;
BsServiceFlow *BsSF_head;
BsServiceFlow *BsSF_tail;
UplinkBurstProfile *UlBurstProfile_head;
UplinkBurstProfile *UlBurstProfile_tail;
DownlinkBurstProfile *DlBurstProfile_head;
DownlinkBurstProfile *DlBurstProfile_tail;
UlMapIe *UlMapIe_head;
UlMapIe *UlMapIe_tail;
DlMapIe *DlMapIe_head;
DlMapIe *DlMapIe_tail;

Mac802_16::Mac802_16() :Mac(),frameTimer(this),preambleTimer(this),downlinkTimer(this),uplinkTimer(this),ulmapTimer(this),dlmapTimer(this)
{
	NodeID = index_;
	if (NodeID == BsID) {
		TransactionID_Counter = 0x8000; //0x8000 ~ 0xFFFFF
		BsSsInfo_head = NULL;
		BsSsInfo_tail = NULL;
		BsSF_head = NULL;
		BsSF_tail = NULL;
		UlBurstProfile_head = NULL;
		UlBurstProfile_tail = NULL;
		DlBurstProfile_head = NULL;
		DlBurstProfile_tail = NULL;
		UlMapIe_head = NULL;
		UlMapIe_tail = NULL;
		DlMapIe_head = NULL;
		DlMapIe_tail = NULL;
		DlAvailSymbol = DlSymbolNum*DlSubchanNum;
		UlAvailSymbol = UlSymbolNum*UlSubchanNum;
		frameTimer.start(0);
		DlMapIeNum = 0;
		UlMapIeNum = 0;
	} else {
		TransactionID_Counter = 0x0000;	//0x0000 ~ 0x7FFFF
		SsSF_head = NULL;
		SsSF_tail = NULL;
		RangingStatus = false;
	}
}  

void 
Mac802_16::recv(Packet* p, Handler* h)
{
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p);
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);	
	UcdMsg *ucd = (UcdMsg*) p->accessdata();
	DcdMsg *dcd = (DcdMsg*) p->accessdata();
	RngReqMsg *rngreq = (RngReqMsg*) p->accessdata();
	RngRspMsg *rngrsp = (RngRspMsg*) p->accessdata();
	DsaReqMsg *dsareq = (DsaReqMsg*) p->accessdata();
	DsaRspMsg *dsarsp = (DsaRspMsg*) p->accessdata();
	DsaAckMsg *dsaack = (DsaAckMsg*) p->accessdata();
	
	if (hdr_cmn::access(p)->direction() == hdr_cmn::UP) {
		if (NodeID == BsID) {
			if (hdr->ptype() == PT_UGS) {			
				if (BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)) {
					/*if (hdr->next_hop() != BsID) {	
						if (!BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_UGS, DL, -1, -1))
							sendDSAREQ(NULL, BsCreateServiceFlow(p, DL));
						BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_UGS, DL, -1, -1)->SduQueue.enque(p);
					} else*/
						uptarget_->recv(p, this);
					//printf("bs  recv UGS pkt from ss%d\n",BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr);
				}
			} else if (hdr->ptype() == PT_ertPS) {	
				if (BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)) {
					/*if (hdr->next_hop() != BsID) {	
						if (!BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_ertPS, DL, -1, -1))
							sendDSAREQ(NULL, BsCreateServiceFlow(p, DL));
						BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_ertPS, DL, -1, -1)->SduQueue.enque(p);
					} else*/
						uptarget_->recv(p, this);
					//printf("bs  recv ertPS pkt from ss%d\n",BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr);
				}				
			} else if (hdr->ptype() == PT_rtPS) { 		
				if (BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)) {
					/*if (hdr->next_hop() != BsID) {	
						if (!BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_rtPS, DL, -1, -1))
							sendDSAREQ(NULL, BsCreateServiceFlow(p, DL));
						BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_rtPS, DL, -1, -1)->SduQueue.enque(p);
					} else*/
						uptarget_->recv(p, this);
					//printf("bs  recv rtPS pkt from ss%d\n",BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr);
				}					
			} else if (hdr->ptype() == PT_nrtPS) {	
				if (BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)) {
					/*if (hdr->next_hop() != BsID) {	
						if (!BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_nrtPS, DL, -1, -1))
							sendDSAREQ(NULL, BsCreateServiceFlow(p, DL));
						BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_nrtPS, DL, -1, -1)->SduQueue.enque(p);
					} else*/
						uptarget_->recv(p, this);
					//printf("bs  recv nrtPS pkt from ss%d\n",BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr);
				}					
			} else if (hdr->ptype() == PT_BE) {
				if (BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)) {
					/*if (hdr->next_hop() != BsID) {	
						if (!BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_BE, DL, -1, -1))
							sendDSAREQ(NULL, BsCreateServiceFlow(p, DL));
						BsSearchServiceFlow(BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr, hdr->next_hop(), PT_BE, DL, -1, -1)->SduQueue.enque(p);
					} else*/
						uptarget_->recv(p, this);	
					//printf("bs  recv BE pkt from ss%d\n",BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, -1, mac16->generic_hdr.CID)->MacAddr);
				}
			} else if (hdr->ptype() == PT_ACK) {
				if (mac->macDA() == NodeID)
					uptarget_->recv(p, this);
			} else if (hdr->ptype() == PT_BWREQ) {		
				//printf("bs  recv BW-REQ from ss%d\n",mac->macSA());
				BsSearchSsInfo(-1, mac16->bwreq_hdr.CID)->UplinkBandwidth += mac16->bwreq_hdr.BR;
				mac_log(p);
			} else if (hdr->ptype() == PT_RNGREQ) {
				//printf("bs  recv RNG-REQ from ss%d \n", mac->macSA());
				mac_log(p->copy());	
				if (!BsSearchSsInfo(rngreq->SS_MAC_Address, -1))					
					sendRNGRSP(BsCreateSsInfo(p));
			} else if (hdr->ptype() == PT_DSAREQ) {
				if (dsareq->Transaction_ID <= 0x7FFF) {
					packet_t ptype;	
					switch (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value) {
						case 0x06: ptype = PT_UGS; break;
						case 0x05: ptype = PT_ertPS; break;
						case 0x04: ptype = PT_rtPS; break;
						case 0x03: ptype = PT_nrtPS; break;
						case 0x02: ptype = PT_BE; break;
					}		
					if (BsSearchSsInfo(-1, mac16->generic_hdr.CID) && !BsSearchServiceFlow(dsareq->Service_Flow_Parameters.Source_MAC_Address.Value, dsareq->Service_Flow_Parameters.Destination_MAC_Address.Value, ptype, UL, -1, -1)) {
						//printf("bs  recv DSA-REQ from ss%d\n", BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr);
						mac_log(p->copy());
						sendDSXRVD(p->copy());
						sendDSARSP(NULL, BsCreateServiceFlow(p->copy(), UL));
					}
				}				
			} else if (hdr->ptype() == PT_DSARSP) {			
				if (dsarsp->Transaction_ID >= 0x8000)
					if (BsSearchSsInfo(-1, mac16->generic_hdr.CID)) {					
						mac_log(p->copy());
						if (dsarsp->Confirmation_Code == 0x00)
							BsSearchServiceFlow(-1, -1, PT_NTYPE, DL, dsarsp->Transaction_ID, -1)->Status = Active;
						sendDSAACK(p);
					}			
			} else if (hdr->ptype() == PT_DSAACK) {	
				if (dsaack->Transaction_ID <= 0x7FFF) {
					if (BsSearchServiceFlow(-1, -1, PT_NTYPE, UL, dsaack->Transaction_ID, -1)) {
						//printf("bs  recv DSA-ACK from ss%d\n",mac->macSA());
						mac_log(p);
					} 
				}		
			} else {		
				sendUp(p);
				return;		
			}
			return;
		} else {
			if (hdr->ptype() == PT_UGS) {
				if (SsSearchServiceFlow(-1, PT_NTYPE, DL, -1, mac16->generic_hdr.CID)) {
					uptarget_->recv(p, this);
					//printf("ss%d recv UGS pkt\n",NodeID);
				}
			} else if (hdr->ptype() == PT_ertPS) {
				if (SsSearchServiceFlow(-1, PT_NTYPE, DL, -1, mac16->generic_hdr.CID)) {
					uptarget_->recv(p, this);
					//printf("ss%d recv ertPS pkt\n",NodeID);
				}
			} else if (hdr->ptype() == PT_rtPS) { 
				if (SsSearchServiceFlow(-1, PT_NTYPE, DL, -1, mac16->generic_hdr.CID)) {
					uptarget_->recv(p, this);
					//printf("ss%d recv rtPS pkt\n",NodeID);
				}
			} else if (hdr->ptype() == PT_nrtPS) {
				if (SsSearchServiceFlow(-1, PT_NTYPE, DL, -1, mac16->generic_hdr.CID)) {
					uptarget_->recv(p, this);
					//printf("ss%d recv nrtPS pkt\n",NodeID);
				}
			} else if (hdr->ptype() == PT_BE) {
				if (SsSearchServiceFlow(-1, PT_NTYPE, DL, -1, mac16->generic_hdr.CID)) {
					uptarget_->recv(p, this);
					//printf("ss%d recv BE pkt\n",NodeID);
				}
			} else if (hdr->ptype() == PT_ACK) {
				if (mac->macDA() == NodeID)
					uptarget_->recv(p, this);
			} else if (hdr->ptype() == PT_UCD) {
				//printf("ss%d recv UCD\n",NodeID);
				if (UcdConfigChangeCount != ucd->Config_Change_Count) {
					UcdConfigChangeCount = ucd->Config_Change_Count;
					UplinkBurstProfile *tmp = UlBurstProfile_head;
					while (tmp) {
						int FEC_Code;
						switch (ModulCoding) {
							case QPSK_12: FEC_Code = 0; break;
							case QPSK_34: FEC_Code = 1; break;
							case QAM16_12: FEC_Code = 2; break;
							case QAM16_34: FEC_Code = 3; break;
							case QAM64_23: FEC_Code = 4; break;
							case QAM64_34: FEC_Code = 5; break;
						}
						if (tmp->FEC_Code_and_modulation_type.Value == FEC_Code) {
							UplinkBurstProfileTable.UIUC = tmp->UIUC;
							UplinkBurstProfileTable.FEC_Code_and_modulation_type.Value = tmp->FEC_Code_and_modulation_type.Value;
						}
						tmp = tmp->next;
					}
				}
				mac_log(p);				
			} else if (hdr->ptype() == PT_DCD) {
				//printf("ss%d recv DCD\n",NodeID);
				if (DcdConfigChangeCount != dcd->Config_Change_Count) {
					DcdConfigChangeCount = dcd->Config_Change_Count;
					DownlinkBurstProfile *tmp = DlBurstProfile_head;
					while (tmp) {
						int FEC_Code;
						ModulCoding = CalMod->search(NodeID)->ToMod(CalMod->search(0)->x,CalMod->search(0)->y);
						switch (ModulCoding) {
							case QPSK_12: FEC_Code = 0; break;
							case QPSK_34: FEC_Code = 1; break;
							case QAM16_12: FEC_Code = 2; break;
							case QAM16_34: FEC_Code = 3; break;
							case QAM64_23: FEC_Code = 4; break;
							case QAM64_34: FEC_Code = 5; break;
						}
						if (tmp->FEC_Code_type.Value == FEC_Code) {
							DownlinkBurstProfileTable.DIUC = tmp->DIUC;
							DownlinkBurstProfileTable.FEC_Code_type.Value = tmp->FEC_Code_type.Value;
						}
						tmp = tmp->next;
					}
				}
				mac_log(p);				
			} else if (hdr->ptype() == PT_BWREQ) {		
				Packet::free(p);				
			} else if (hdr->ptype() == PT_RNGREQ) {			
				Packet::free(p);
			} else if (hdr->ptype() == PT_DLMAP) {			
				mac_log(p);			
			} else if (hdr->ptype() == PT_ULMAP) {			
				ulmapTimer.start(AllocationStartTime - Scheduler::instance().clock());
				mac_log(p);				
			} else if (hdr->ptype() == PT_RNGRSP) {				
				if ((int)rngrsp->SS_MAC_Address == NodeID) {
					//printf("ss%d recv RNG-RSP basic=%d primary=%d\n",NodeID,rngrsp->Basic_CID,rngrsp->Primary_Management_CID);					
					if (rngrsp->Ranging_Status == 0x03) {
						RangingStatus = true;
						BasicCID = rngrsp->Basic_CID;
						PrimaryCID = rngrsp->Primary_Management_CID;	
					} 
					mac_log(p);
				}			
			} else if (hdr->ptype() == PT_DSAREQ) {		
				if (dsareq->Transaction_ID >= 0x8000) {
					packet_t ptype;	
					switch (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value) {
						case 0x06: ptype = PT_UGS; break;
						case 0x05: ptype = PT_ertPS; break;
						case 0x04: ptype = PT_rtPS; break;
						case 0x03: ptype = PT_nrtPS; break;
						case 0x02: ptype = PT_BE; break;
					}		
					if (mac16->generic_hdr.CID == PrimaryCID) {
						//printf("ss%d recv DSA-REQ\n",NodeID);
						mac_log(p->copy());
						if (!SsSearchServiceFlow(dsareq->Service_Flow_Parameters.Destination_MAC_Address.Value, ptype, DL, -1, -1))
							sendDSARSP(SsCreateServiceFlow(p, DL), NULL);
					}		
				}			
			} else if (hdr->ptype() == PT_DSXRVD) {			
				if (mac16->generic_hdr.CID == PrimaryCID)				
					mac_log(p);			
			} else if (hdr->ptype() == PT_DSARSP) {				
				if (dsarsp->Transaction_ID <= 0x7FFF) {
					if (mac16->generic_hdr.CID == PrimaryCID) {					
						mac_log(p->copy());
						if (dsarsp->Confirmation_Code == 0x00) {
							SsSearchServiceFlow(-1, PT_NTYPE, UL, dsarsp->Transaction_ID, -1)->Status = Active;
							SsSearchServiceFlow(-1, PT_NTYPE, UL, dsarsp->Transaction_ID, -1)->TransportCID = dsarsp->Service_Flow_Parameters.CID.Value;
							SsSearchServiceFlow(-1, PT_NTYPE, UL, dsarsp->Transaction_ID, -1)->SFID = dsarsp->Service_Flow_Parameters.SFID.Value;
						}
						sendDSAACK(p);
					}
				}			
			} else if (hdr->ptype() == PT_DSAACK) {			
				if (dsaack->Transaction_ID >= 0x8000)
					if (mac16->generic_hdr.CID == PrimaryCID)
						mac_log(p);
			} else {	
				sendUp(p);
				return;			
			}
			return;
		}
	} else {
		callback_ = h;
		if (NodeID == BsID) {					
			if (hdr->ptype() == PT_UGS) {
				if (!BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_UGS, DL, -1, -1))
					BsCreateServiceFlow(p, DL);
				if (BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_UGS, DL, -1, -1)->SduQueue.length() == 0)
					BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_UGS, DL, -1, -1)->QueueTime = Scheduler::instance().clock();	
				BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_UGS, DL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_ertPS) {
				if (!BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_ertPS, DL, -1, -1))
					BsCreateServiceFlow(p, DL);
				if (BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_ertPS, DL, -1, -1)->SduQueue.length() == 0)
					BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_ertPS, DL, -1, -1)->QueueTime = Scheduler::instance().clock();	
				BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_ertPS, DL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_rtPS) { 
				if (!BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_rtPS, DL, -1, -1))
					BsCreateServiceFlow(p, DL);
				if (BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_rtPS, DL, -1, -1)->SduQueue.length() == 0)
					BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_rtPS, DL, -1, -1)->QueueTime = Scheduler::instance().clock();	
				BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_rtPS, DL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_nrtPS) {
				if (!BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_nrtPS, DL, -1, -1))
					BsCreateServiceFlow(p, DL);
				if (BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_nrtPS, DL, -1, -1)->SduQueue.length() == 0)
					BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_nrtPS, DL, -1, -1)->QueueTime = Scheduler::instance().clock();	
				BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_nrtPS, DL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_BE) {	
				if (!BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_BE, DL, -1, -1))
					BsCreateServiceFlow(p, DL);
				if (BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_BE, DL, -1, -1)->SduQueue.length() == 0)
					BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_BE, DL, -1, -1)->QueueTime = Scheduler::instance().clock();	
				BsSearchServiceFlow(mac->macSA(), mac->macDA(), PT_BE, DL, -1, -1)->SduQueue.enque(p);
			} else {
				mac->set(MF_DATA, NodeID);
				state(MAC_SEND);
				sendDown(p);
				return;
			}
			resume (NULL);	
			if (BsSearchServiceFlow(mac->macSA(), mac->macDA(), hdr->ptype(), DL, -1, -1)->Status == Provisioned) {
				sendDSAREQ(NULL, BsSearchServiceFlow(mac->macSA(), mac->macDA(), hdr->ptype(), DL, -1, -1));
				BsSearchServiceFlow(mac->macSA(), mac->macDA(), hdr->ptype(), DL, -1, -1)->Status = Admitted;
			}
		} else {
			if (hdr->ptype() == PT_UGS) {
				if (!SsSearchServiceFlow(mac->macDA(), PT_UGS, UL, -1, -1))
					SsCreateServiceFlow(p, UL);
				if (SsSearchServiceFlow(mac->macDA(), PT_UGS, UL, -1, -1)->SduQueue.length() == 0)
					SsSearchServiceFlow(mac->macDA(), PT_UGS, UL, -1, -1)->QueueTime = Scheduler::instance().clock();	
				SsSearchServiceFlow(mac->macDA(), PT_UGS, UL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_ertPS) {
				if (!SsSearchServiceFlow(mac->macDA(), PT_ertPS, UL, -1, -1))
					SsCreateServiceFlow(p, UL);
				if (SsSearchServiceFlow(mac->macDA(), PT_ertPS, UL, -1, -1)->SduQueue.length() == 0)
					SsSearchServiceFlow(mac->macDA(), PT_ertPS, UL, -1, -1)->QueueTime = Scheduler::instance().clock();
				SsSearchServiceFlow(mac->macDA(), PT_ertPS, UL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_rtPS) { 
				if (!SsSearchServiceFlow(mac->macDA(), PT_rtPS, UL, -1, -1))
					SsCreateServiceFlow(p, UL);
				if (SsSearchServiceFlow(mac->macDA(), PT_rtPS, UL, -1, -1)->SduQueue.length() == 0)
					SsSearchServiceFlow(mac->macDA(), PT_rtPS, UL, -1, -1)->QueueTime = Scheduler::instance().clock();
				SsSearchServiceFlow(mac->macDA(), PT_rtPS, UL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_nrtPS) {
				if (!SsSearchServiceFlow(mac->macDA(), PT_nrtPS, UL, -1, -1))
					SsCreateServiceFlow(p, UL);
				if (SsSearchServiceFlow(mac->macDA(), PT_nrtPS, UL, -1, -1)->SduQueue.length() == 0)
					SsSearchServiceFlow(mac->macDA(), PT_nrtPS, UL, -1, -1)->QueueTime = Scheduler::instance().clock();
				SsSearchServiceFlow(mac->macDA(), PT_nrtPS, UL, -1, -1)->SduQueue.enque(p);
			} else if (hdr->ptype() == PT_BE) {	
				if (!SsSearchServiceFlow(mac->macDA(), PT_BE, UL, -1, -1))
					SsCreateServiceFlow(p, UL);
				if (SsSearchServiceFlow(mac->macDA(), PT_BE, UL, -1, -1)->SduQueue.length() == 0)
					SsSearchServiceFlow(mac->macDA(), PT_BE, UL, -1, -1)->QueueTime = Scheduler::instance().clock();
				SsSearchServiceFlow(mac->macDA(), PT_BE, UL, -1, -1)->SduQueue.enque(p);
			} else {
				mac->set(MF_DATA, NodeID);
				state(MAC_SEND);
				sendDown(p);
				return;
			}
			resume (NULL);
			if (RangingStatus && SsSearchServiceFlow(mac->macDA(), hdr->ptype(), UL, -1, -1)->Status == Provisioned) {
				sendDSAREQ(SsSearchServiceFlow(mac->macDA(), hdr->ptype(), UL, -1, -1), NULL);
				SsSearchServiceFlow(mac->macDA(), hdr->ptype(), UL, -1, -1)->Status = Admitted;
			}
		}
	}
}

void 
Mac802_16::sendUCD()
{
	printf("bs  send UCD\n");
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct UcdMsg));	
	
	mac->macDA() = -1;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
    hdr->ptype() = PT_UCD;
    hdr->size() = sizeof(GenericHdr) + sizeof(UcdMsg) + MaxProfileNum*sizeof(UplinkBurstProfile)/2;
    hdr->iface() = -2;
    hdr->error() = 0; 
	hdr->txtime() = 2*SymbolLength;
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = 0xFFFF;
	mac16->generic_hdr.HCS = 0;
	
	UcdMsg *ucd = (UcdMsg*) p->accessdata();
	ucd->Management_Message_Type = 0x00;
	ucd->Config_Change_Count = 0x01;
	ucd->Rng_Backoff_Start = 0x00;
	ucd->Rng_Backoff_End = 0x0F;
	ucd->Req_Backoff_Start = 0x00;
	ucd->Req_Backoff_End = 0x0F;
	for (int i = 0; i < MaxProfileNum; i++) {
		UplinkBurstProfile *tmp = new UplinkBurstProfile;
		tmp->Type = 0x01;
		tmp->Length = sizeof(UplinkBurstProfile);
		tmp->UIUC = i+1;
		tmp->FEC_Code_and_modulation_type.Value = i;
		if (UlBurstProfile_head == NULL) {
			UlBurstProfile_head = UlBurstProfile_tail = tmp;
			tmp->next = NULL;
		} else {
			UlBurstProfile_tail->next = tmp;
			UlBurstProfile_tail = tmp;
			tmp->next = NULL;
		}
	}
	/*Number of UL-MAPs to receive before contention-based reservation is attempted again for the same connection.*/
	ucd->Contention_based_reservation_timeout.Value = 0x00;
	/*Size (in units of PS) of PHY payload that SS may use to format and transmit a bandwidth request message in a contention request opportunity.
	    The value includes all PHY overhead as well as allowance for the MAC data the message may hold.*/
	ucd->Bandwidth_request_opportunity_size.Value = 0x00;
	/*Size (in units of PS) of PHY bursts that an SS may use to transmit a RNG-REQ message in a contention ranging request opportunity.
	    The value includes all PHY overhead as well as the maximum SS/BS round trip propagation delay.*/
	ucd->Ranging_request_opportunity_size.Value = 0x00;
	
	downtarget_->recv(p,this);
	return;
}

void 
Mac802_16::sendDCD()
{
	printf("bs  send DCD\n");
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct DcdMsg));	
	
	mac->macDA() = -1;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
    hdr->ptype() = PT_DCD;
    hdr->size() = sizeof(GenericHdr) + sizeof(DcdMsg) + MaxProfileNum*sizeof(DownlinkBurstProfile)/2;
    hdr->iface() = -2;
    hdr->error() = 0; 
	hdr->txtime() = 2*SymbolLength;
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = 0xFFFF;
	mac16->generic_hdr.HCS = 0;
	
	DcdMsg *dcd = (DcdMsg*) p->accessdata();
	dcd->Management_Message_Type = 0x00;
	dcd->Config_Change_Count = 0x01;

	for (int i = 0; i < MaxProfileNum; i++) {
		DownlinkBurstProfile *tmp = new DownlinkBurstProfile;
		tmp->DIUC = i+1;
		tmp->FEC_Code_type.Value = i;
		if (DlBurstProfile_head == NULL) {
			DlBurstProfile_head = DlBurstProfile_tail = tmp;
			tmp->next = NULL;
		} else {
			DlBurstProfile_tail->next = tmp;
			DlBurstProfile_tail = tmp;
			tmp->next = NULL;
		}
	}
	/*Base Station ID.*/
	dcd->BS_ID.Value = NodeID;
	/*See 11.1.3.*/
	dcd->MAC_version.Value = 0x04;
	
	downtarget_->recv(p,this);
	return;
}

void
Mac802_16::sendDLMAP()
{
	printf("bs  send DL-MAP\n");
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct DlMapMsg));
	
	mac->macDA() = -1;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
	hdr->ptype() = PT_DLMAP;
	hdr->size() = sizeof(GenericHdr) + sizeof(DlMapMsg) + DlMapIeNum*sizeof(DlMapIe)/2;
	hdr->iface() = -2;
	hdr->error() = 0; 
	hdr->txtime() =  2*SymbolLength;
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = 0xFFFF;
	mac16->generic_hdr.HCS = 0;
	
	DlMapMsg *dlmap = (DlMapMsg*) p->accessdata();
	dlmap->Management_Message_Type = 2;
	dlmap->DCD_Count = DCD_Counter;
	dlmap->BS_ID = BsID;
	dlmap->No_OFDMA_Symbols = DlSymbolNum;
	dlmap->Frame_duration_code = 0x04;
	dlmap->Frame_number = FrameNumber_Counter;
	
	downtarget_->recv(p,this);
	return;
}

void
Mac802_16::sendULMAP()
{
	printf("bs  send UL-MAP\n");
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct UlMapMsg));
	
	mac->macDA() = -1;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
	hdr->ptype() = PT_ULMAP;
	hdr->size() = sizeof(GenericHdr) + sizeof(UlMapMsg) + UlMapIeNum*sizeof(UlMapIe)/2;
	hdr->iface() = -2;
	hdr->error() = 0; 
	hdr->txtime() = 2*SymbolLength;
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = 0xFFFF;
	mac16->generic_hdr.HCS = 0;
	
	UlMapMsg *ulmap = (UlMapMsg*) p->accessdata();
	ulmap->Management_Message_Type = 0x03;
	ulmap->UCD_Count = UCD_Counter;
	ulmap->Allocation_Start_Time = (u_int32_t)Scheduler::instance().clock();
	AllocationStartTime = Scheduler::instance().clock() //Current time
							+ SymbolLength //Preamble interval
							+ DlSymbolNum*SymbolLength //Downlink interval
							+ TtgLength; //TTG interval

	downtarget_->recv(p,this);
	return;
}

void 
Mac802_16::sendRNGREQ(UlMapIe *IE)
{ 
	printf("ss%d send RNG-REQ\n", NodeID);
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct RngReqMsg));
	
	//ModulCoding = CalMod->search(NodeID)->ToMod(CalMod->search(0)->x,CalMod->search(0)->y);
	mac->macDA() = BsID;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
    hdr->ptype() = PT_RNGREQ;
    hdr->size() = sizeof(GenericHdr) + sizeof(RngReqMsg);
    hdr->iface() = -2;
    hdr->error() = 0; 
	hdr->txtime() = txtime(hdr->size());
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = 0x0000;
	mac16->generic_hdr.HCS = 0;

	RngReqMsg *rngreq = (RngReqMsg*) p->accessdata();
	rngreq->Management_Message_Type = 0x04;
	rngreq->Requested_Downlink_Burst_Profile = DownlinkBurstProfileTable.DIUC;
	rngreq->SS_MAC_Address = NodeID;
	rngreq->Ranging_Anomalies = 0;
	rngreq->AAS_broadcast_capability = 0;
	
	uplinkTimer.start(IE->OFDMA_Symbol_offset*SymbolLength, p);
	return;
}

void 
Mac802_16::sendBWREQ(UlMapIe *IE)
{
	printf("ss%d send BW-REQ\n", NodeID);
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	
	mac->macDA() = BsID;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
    hdr->ptype() = PT_BWREQ;
    hdr->size() = sizeof(BwReqHdr);
    hdr->iface() = -2;
    hdr->error() = 0; 
	hdr->txtime() = txtime(hdr->size());
	
	mac16->bwreq_hdr.HT = 1;
	mac16->bwreq_hdr.EC = 1;
	mac16->bwreq_hdr.Type = 1;
	SsServiceFlow *sssf;

	sssf = SsSF_head;
	while (sssf) {
		//ertPS Bandwidth Request
		if (sssf->ScheduleType == 5)
			if (Scheduler::instance().clock() - sssf->QueueTime >= (double)sssf->ToleratedJitter/1000 ) {
				mac16->bwreq_hdr.BR += sssf->SduQueue.byteLength(); 
				sssf->BwReqSize = sssf->SduQueue.byteLength(); 
			}
		//rtPS and nrtPS Bandwidth Request
		if (sssf->ScheduleType >= 3 && sssf->ScheduleType <= 4)
			if (Scheduler::instance().clock() - sssf->QueueTime >= (double)sssf->MaxLatency/1000 ) {
				mac16->bwreq_hdr.BR += sssf->SduQueue.byteLength(); 
				sssf->BwReqSize = sssf->SduQueue.byteLength(); 
			}
		sssf = sssf->next;
	}
	mac16->bwreq_hdr.CID = BasicCID;
	mac16->bwreq_hdr.HCS = 0;
	uplinkTimer.start(IE->OFDMA_Symbol_offset*SymbolLength, p);
	return;
}

void
Mac802_16::sendRNGRSP(BsSsInfo *SsInfo)
{
	printf("bs  send RNG-RSP to ss%d\n", SsInfo->MacAddr);
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct RngRspMsg));
	
	mac->macDA() = SsInfo->MacAddr;
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
    hdr->ptype() = PT_RNGRSP;
    hdr->size() = sizeof(GenericHdr) + sizeof(RngRspMsg);	
    hdr->iface() = -2;
    hdr->error() = 0; 
	hdr->txtime() = txtime(hdr->size());
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = 0x0000;
	mac16->generic_hdr.HCS = 0;
	
	RngRspMsg *rngrsp = (RngRspMsg*) p->accessdata();
	rngrsp->Timing_Adjust = 0;
	rngrsp->Power_Level_Adjust = 0;
	rngrsp->Offset_Frequency_Adjust = 0;
	rngrsp->Ranging_Status = 3;
	rngrsp->Downlink_frequency_override = 0;
	rngrsp->Uplink_channel_ID_override = 0;
	rngrsp->Downlink_Operational_Burst_Profile = 0;
	rngrsp->SS_MAC_Address = SsInfo->MacAddr;
	rngrsp->Basic_CID = SsInfo->BasicCID;
	rngrsp->Primary_Management_CID = SsInfo->PrimaryCID;
	rngrsp->AAS_broadcast_permission = 0;
	
	downtarget_->recv(p,this);
	return;
}

void 
Mac802_16::sendDSAREQ(SsServiceFlow *SsSF, BsServiceFlow *BsSF)
{ 
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p); 
	p->allocdata (sizeof (struct DsaReqMsg));	
		
	if(NodeID == BsID) {
		printf("bs  send DSA-REQ to ss%d\n", BsSF->MacAddr);
		
		mac->macDA() = BsSF->MacAddr;
		mac->macSA() = NodeID;
		mac->set(MF_DATA, NodeID);
		state(MAC_SEND);
	
		hdr->uid() = 0;
		hdr->ptype() = PT_DSAREQ;
		hdr->size() = sizeof(GenericHdr) + sizeof(DsaReqMsg);
		hdr->iface() = -2;
		hdr->error() = 0; 
		hdr->txtime() = txtime(hdr->size());
	
		mac16->generic_hdr.HT = 0;
		mac16->generic_hdr.EC = 1;
		mac16->generic_hdr.Type = 1;
		mac16->generic_hdr.CI = 1; 
		mac16->generic_hdr.EKS = 1;
		mac16->generic_hdr.LEN = 3;
		mac16->generic_hdr.CID = BsSearchSsInfo(BsSF->MacAddr, -1)->PrimaryCID;
		mac16->generic_hdr.HCS = 0;
		
		DsaReqMsg *dsareq = (DsaReqMsg*) p->accessdata();	
		dsareq->Management_Message_Type = 11;
		dsareq->Transaction_ID = BsSF->TransactionID;
		dsareq->Service_Flow_Parameters.CID.Value = BsSF->TransportCID;
		dsareq->Service_Flow_Parameters.SFID.Value = BsSF->SFID;
		dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value = BsSF->ScheduleType;
		if (BsSF->Status == Provisioned) {
			dsareq->Service_Flow_Parameters.QoS_Parameter_Set_Type.Value = 1;
		} else if (BsSF->Status == Admitted) {
			dsareq->Service_Flow_Parameters.QoS_Parameter_Set_Type.Value = 2;
		} else if (BsSF->Status == Active) {
			dsareq->Service_Flow_Parameters.QoS_Parameter_Set_Type.Value = 4;
		}
		dsareq->Service_Flow_Parameters.Traffic_Priority.Value = BsSF->TrafficPriority;
		dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value = BsSF->MaxTrafficRate;
		dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value = BsSF->MinTrafficRate;
		dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value = BsSF->ScheduleType;
		dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value = BsSF->ReqTxPolicy;
		dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value = BsSF->ToleratedJitter;
		dsareq->Service_Flow_Parameters.Maximum_Latency.Value = BsSF->MaxLatency;
		dsareq->Service_Flow_Parameters.Destination_MAC_Address.Value = BsSF->DstMacAddr;
		dsareq->Service_Flow_Parameters.Source_MAC_Address.Value = BsSF->SrcMacAddr;
	} else {
		printf("ss%d send DSA-REQ\n", NodeID);
		
		mac->macDA() = BsID;
		mac->macSA() = NodeID;
		mac->set(MF_DATA, NodeID);
		state(MAC_SEND);
	
		hdr->uid() = 0;
		hdr->ptype() = PT_DSAREQ;
		hdr->size() = sizeof(GenericHdr) + sizeof(DsaReqMsg);
		hdr->iface() = -2;
		hdr->error() = 0; 
		hdr->txtime() = txtime(hdr->size());
	
		mac16->generic_hdr.HT = 0;
		mac16->generic_hdr.EC = 1;
		mac16->generic_hdr.Type = 1;
		mac16->generic_hdr.CI = 1; 
		mac16->generic_hdr.EKS = 1;
		mac16->generic_hdr.LEN = 3;
		mac16->generic_hdr.CID = PrimaryCID;
		mac16->generic_hdr.HCS = 0;
		
		DsaReqMsg *dsareq = (DsaReqMsg*) p->accessdata();	
		dsareq->Management_Message_Type = 11;
		dsareq->Transaction_ID = SsSF->TransactionID;
		dsareq->Service_Flow_Parameters.SFID.Value = 0;
		if (SsSF->Status == Provisioned) {
			dsareq->Service_Flow_Parameters.QoS_Parameter_Set_Type.Value = 1;
		} else if (SsSF->Status == Admitted) {
			dsareq->Service_Flow_Parameters.QoS_Parameter_Set_Type.Value = 2;
		} else if (SsSF->Status == Active) {
			dsareq->Service_Flow_Parameters.QoS_Parameter_Set_Type.Value = 4;
		}
		dsareq->Service_Flow_Parameters.Traffic_Priority.Value = SsSF->TrafficPriority;
		dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value = SsSF->MaxTrafficRate;
		dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value = SsSF->MinTrafficRate;
		dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value = SsSF->ScheduleType;
		dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value = SsSF->ReqTxPolicy;
		dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value = SsSF->ToleratedJitter;
		dsareq->Service_Flow_Parameters.Maximum_Latency.Value = SsSF->MaxLatency;
		dsareq->Service_Flow_Parameters.Destination_MAC_Address.Value = SsSF->DstMacAddr;
		dsareq->Service_Flow_Parameters.Source_MAC_Address.Value = SsSF->SrcMacAddr;
	}
	if (Scheduler::instance().clock() >= AllocationStartTime && Scheduler::instance().clock() <= AllocationStartTime + UlSymbolNum*SymbolLength) {
		downtarget_->recv(p, this);
	} else {
		if (NodeID == BsID) {
			if (Scheduler::instance().clock() < AllocationStartTime)
				downlinkTimer.start(AllocationStartTime - Scheduler::instance().clock(), p);
			else
				downlinkTimer.start(Scheduler::instance().clock() - AllocationStartTime, p);
		} else {
			if (Scheduler::instance().clock() < AllocationStartTime)
				uplinkTimer.start(AllocationStartTime - Scheduler::instance().clock(), p);
			else
				uplinkTimer.start(Scheduler::instance().clock() - AllocationStartTime, p);
		}
	}
	return;
}

void
Mac802_16::sendDSARSP(SsServiceFlow *SsSF, BsServiceFlow *BsSF)
{
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct DsaRspMsg));
	
	if(NodeID == BsID) {
		printf("bs  send DSA-RSP to ss%d\n", BsSF->MacAddr);
		
		mac->macDA() = BsSF->MacAddr;
		mac->macSA() = NodeID;
		mac->set(MF_DATA, NodeID);
		state(MAC_SEND);
	
		hdr->uid() = 0;
		hdr->ptype() = PT_DSARSP;
		hdr->size() = sizeof(GenericHdr) + sizeof(DsaRspMsg);
		hdr->iface() = -2;
		hdr->error() = 0; 
		hdr->txtime() = txtime(hdr->size());
	
		mac16->generic_hdr.HT = 0;
		mac16->generic_hdr.EC = 1;
		mac16->generic_hdr.Type = 1;
		mac16->generic_hdr.CI = 1; 
		mac16->generic_hdr.EKS = 1;
		mac16->generic_hdr.LEN = 3;
		mac16->generic_hdr.CID = BsSearchSsInfo(BsSF->MacAddr, -1)->PrimaryCID;
		mac16->generic_hdr.HCS = 0;
		
		DsaRspMsg *dsarsp = (DsaRspMsg*) p->accessdata();
		dsarsp->Management_Message_Type = 30;
		dsarsp->Transaction_ID = BsSF->TransactionID;
		dsarsp->Confirmation_Code = 0x00;	
		dsarsp->Service_Flow_Parameters.CID.Value = BsSF->TransportCID;
		dsarsp->Service_Flow_Parameters.SFID.Value = BsSF->SFID;
	} else {
		printf("ss%d send DSA-RSP\n", NodeID);
		
		mac->macDA() = BsID;
		mac->macSA() = NodeID;
		mac->set(MF_DATA, NodeID);
		state(MAC_SEND);
	
		hdr->uid() = 0;
		hdr->ptype() = PT_DSARSP;
		hdr->size() = sizeof(GenericHdr) + sizeof(DsaRspMsg);
		hdr->iface() = -2;
		hdr->error() = 0; 
		hdr->txtime() = txtime(hdr->size());
	
		mac16->generic_hdr.HT = 0;
		mac16->generic_hdr.EC = 1;
		mac16->generic_hdr.Type = 1;
		mac16->generic_hdr.CI = 1; 
		mac16->generic_hdr.EKS = 1;
		mac16->generic_hdr.LEN = 3;
		mac16->generic_hdr.CID = PrimaryCID;
		mac16->generic_hdr.HCS = 0;
		
		DsaRspMsg *dsarsp = (DsaRspMsg*) p->accessdata();
		dsarsp->Management_Message_Type = 30;
		dsarsp->Transaction_ID = SsSF->TransactionID;
		dsarsp->Confirmation_Code = 0x00;	
		dsarsp->Service_Flow_Parameters.CID.Value = SsSF->TransportCID;
		dsarsp->Service_Flow_Parameters.SFID.Value = SsSF->SFID;
	}
	downtarget_->recv(p,this);
	return;
}

void 
Mac802_16::sendDSAACK(Packet *pkt)
{ 
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	struct hdr_mac802_16 *mac16_tmp = HDR_MAC802_16(pkt);
	p->allocdata (sizeof (struct DsaAckMsg));
	
	if (NodeID == BsID) {
		printf("bs  send DSX-ACK to ss%d\n",BsSearchSsInfo(-1, mac16_tmp->generic_hdr.CID)->MacAddr);
		mac->macDA() = BsSearchSsInfo(-1, mac16_tmp->generic_hdr.CID)->MacAddr;
		mac->macSA() = NodeID;
	} else {
		printf("ss%d send DSX-ACK\n", NodeID);
		mac->macDA() = NodeID;
		mac->macSA() = BsID;
	}
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
	hdr->ptype() = PT_DSAACK;
	hdr->size() = sizeof(GenericHdr) + sizeof(DsxRvdMsg);;
	hdr->iface() = -2;
	hdr->error() = 0; 
	hdr->txtime() = txtime(hdr->size());
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = mac16_tmp->generic_hdr.CID;
	mac16->generic_hdr.HCS = 0;
	
	DsaAckMsg *dsaack = (DsaAckMsg*) p->accessdata();
	DsaRspMsg *dsarsp = (DsaRspMsg*) pkt->accessdata();
	dsaack->Management_Message_Type = 16;
	dsaack->Transaction_ID = dsarsp->Transaction_ID;
	dsaack->Confirmation_Code = 0x00;

	downtarget_->recv(p,this);
	return;
}

void
Mac802_16::sendDSXRVD(Packet *pkt)
{
	Packet *p = Packet::alloc();
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	struct hdr_mac *mac_tmp = HDR_MAC(pkt);
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);
	struct hdr_mac802_16 *mac16_tmp = HDR_MAC802_16(pkt);
	p->allocdata (sizeof (struct DsxRvdMsg));
	printf("bs  send DSX-RVD to ss%d\n", mac_tmp->macSA());
	
	mac->macDA() = mac_tmp->macSA();
	mac->macSA() = NodeID;
	mac->set(MF_DATA, NodeID);
	state(MAC_SEND);
	
	hdr->uid() = 0;
	hdr->ptype() = PT_DSXRVD;
	hdr->size() = sizeof(GenericHdr) + sizeof(DsxRvdMsg);;
	hdr->iface() = -2;
	hdr->error() = 0; 
	hdr->txtime() = txtime(hdr->size());
	
	mac16->generic_hdr.HT = 0;
	mac16->generic_hdr.EC = 1;
	mac16->generic_hdr.Type = 1;
	mac16->generic_hdr.CI = 1; 
	mac16->generic_hdr.EKS = 1;
	mac16->generic_hdr.LEN = 3;
	mac16->generic_hdr.CID = mac16_tmp->generic_hdr.CID;
	mac16->generic_hdr.HCS = 0;
	
	DsxRvdMsg *dsxrvd = (DsxRvdMsg*) p->accessdata();
	DsaReqMsg *dsareq = (DsaReqMsg*) pkt->accessdata();
	dsxrvd->Management_Message_Type = 30;
	dsxrvd->Transaction_ID = dsareq->Transaction_ID;
	dsxrvd->Confirmation_Code = 0x00;
	
	downtarget_->recv(p,this); 
	return;
}

bool
Mac802_16::CallAdmissionControl(Packet *p)
{
	return true;
}

void
Mac802_16::BandwidthManagement()
{
	//printf("----- %d -----\n",FrameNumber_Counter);
	DlAvailSymbol = (DlSymbolNum-2)*DlSubchanNum;
	UlAvailSymbol = UlSymbolNum*UlSubchanNum;
	//printf("[1] Dl %d Ul %d\n",DlAvailSymbol,UlAvailSymbol);
	int OFDMA_Symbol_offset = 0;
	int Subchannel_offset = 0;
	int BytePerSuchan = 0;
	int UIUC = 0;
	int DIUC = 0;
	UlMapIe *ulmapie = NULL;
	DlMapIe *dlmapie = NULL;
	BsServiceFlow *bssf;
	BsSsInfo *ssinfo;
	ModulCodingType modulcoding = QPSK_12;
	DlMapIeNum = 0;
	UlMapIeNum = 0;
	
	//Create Data Grant IE
	bssf = BsSF_head;
	while (bssf) {
		if (bssf->Direction == DL && bssf->ScheduleType == 6)
			BsSearchSsInfo(bssf->MacAddr, -1)->DownlinkBandwidth += bssf->MaxTrafficRate/8/200;
		if (bssf->Direction == DL && bssf->ScheduleType == 5)	
			if (Scheduler::instance().clock() - bssf->QueueTime >= (double)bssf->ToleratedJitter/1000 ) {
				BsSearchSsInfo(bssf->MacAddr, -1)->DownlinkBandwidth += bssf->SduQueue.byteLength();
				bssf->BwReqSize = bssf->SduQueue.byteLength();
			}
		if (bssf->Direction == DL && bssf->ScheduleType >= 3 && bssf->ScheduleType <= 4)
			if (Scheduler::instance().clock() - bssf->QueueTime >= (double)bssf->MaxLatency/1000 ) {
				BsSearchSsInfo(bssf->MacAddr, -1)->DownlinkBandwidth += bssf->SduQueue.byteLength(); 
				bssf->BwReqSize = bssf->SduQueue.byteLength();
			}
		if (bssf->Direction == DL && bssf->ScheduleType == 2)
			if (Scheduler::instance().clock() - bssf->QueueTime >= (double)bssf->MaxLatency/1000) {
				BsSearchSsInfo(bssf->MacAddr, -1)->DownlinkBandwidth += bssf->MaxTrafficRate/8/200;
				bssf->BwReqSize = bssf->MaxTrafficRate/8/200;
			}
		bssf = bssf->next;
	}
	
	OFDMA_Symbol_offset = 2;
	modulcoding = QPSK_12;
	BytePerSuchan = 6;
	DIUC = 1;
	ssinfo = BsSsInfo_head;
	while (ssinfo) {
		if (ssinfo->ModulCoding == modulcoding && ssinfo->DownlinkBandwidth != 0 && DlAvailSymbol != 0){
			dlmapie = CreateDlMapIe();
			dlmapie->CID = ssinfo->BasicCID;
			dlmapie->DIUC = DIUC;			
			dlmapie->No_OFDMA_Symbols = ssinfo->DownlinkBandwidth/BytePerSuchan/DlSubchanNum/2;
			if ((double)ssinfo->DownlinkBandwidth/BytePerSuchan/DlSubchanNum/2 - dlmapie->No_OFDMA_Symbols > 0)
				dlmapie->No_OFDMA_Symbols += 1;
			dlmapie->No_Subchannels = DlSubchanNum;
			if (DlAvailSymbol < dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*2)
				dlmapie->No_OFDMA_Symbols = DlAvailSymbol/DlSubchanNum/2;
			if (dlmapie->No_OFDMA_Symbols == 0)
				dlmapie->No_OFDMA_Symbols = 1;
			DlAvailSymbol -= dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*2;
			dlmapie->OFDMA_Symbol_offset = OFDMA_Symbol_offset;
			dlmapie->Subchannel_offset = 0;
			dlmapie->Boosting = 0;
			dlmapie->Repetition_Coding_Indication = 0;
			OFDMA_Symbol_offset += dlmapie->No_OFDMA_Symbols;
			DlMapIeNum++;		
		}
		ssinfo = ssinfo->next;
		if (!ssinfo) {
			switch (modulcoding) {
				case QPSK_12: ssinfo = BsSsInfo_head; modulcoding = QPSK_34; BytePerSuchan = 9; DIUC = 2; break;
				case QPSK_34: ssinfo = BsSsInfo_head; modulcoding = QAM16_12; BytePerSuchan = 12; DIUC = 3; break;
				case QAM16_12:ssinfo = BsSsInfo_head; modulcoding = QAM16_34; BytePerSuchan = 18; DIUC = 4; break;
				case QAM16_34:ssinfo = BsSsInfo_head; modulcoding = QAM64_23; BytePerSuchan = 24; DIUC = 5; break;
				case QAM64_23:ssinfo = BsSsInfo_head; modulcoding = QAM64_34; BytePerSuchan = 27; DIUC = 6; break;
				case QAM64_34:break;	
			}
		}
	}
	
	//Create Initial Ranging IE
	Subchannel_offset = 0;
	ulmapie = CreateUlMapIe();
	ulmapie->CID = 0xFFFF;
	ulmapie->UIUC = 0xC;
	ulmapie->No_OFDMA_Symbols = UlSymbolNum;
	ulmapie->No_Subchannels = 0x01;
	ulmapie->Ranging_Method = 0x00;
	ulmapie->OFDMA_Symbol_offset = 0x00;
	ulmapie->Subchannel_offset = 0x00;
	UlAvailSymbol -= ulmapie->No_OFDMA_Symbols;
	Subchannel_offset += ulmapie->No_Subchannels;
	UlMapIeNum++;
	
	//Create BW Request IE
	OFDMA_Symbol_offset = 0;
	bssf = BsSF_head;
	while (bssf) {
		if (bssf->Direction == UL && bssf->ScheduleType >= 3 && bssf->ScheduleType <= 5) {
			if (!BsSearchSsInfo(bssf->MacAddr, -1)->PollingStatus && Scheduler::instance().clock() - bssf->LastPollingTime >= (double)bssf->PollingInterval/1000) {
				bssf->LastPollingTime = Scheduler::instance().clock();
				BsSearchSsInfo(bssf->MacAddr, -1)->PollingStatus = true;
				ulmapie = CreateUlMapIe();
				ulmapie->CID = BsSearchSsInfo(bssf->MacAddr, -1)->BasicCID;
				ulmapie->UIUC = 0xC;
				ulmapie->No_OFDMA_Symbols = 0x02;
				ulmapie->No_Subchannels = 0x01;	
				ulmapie->Ranging_Method = 0x00;
				UlAvailSymbol -= ulmapie->No_OFDMA_Symbols;
				if (OFDMA_Symbol_offset < UlSymbolNum) {
					ulmapie->OFDMA_Symbol_offset = OFDMA_Symbol_offset;
					ulmapie->Subchannel_offset = Subchannel_offset;
					OFDMA_Symbol_offset += ulmapie->No_OFDMA_Symbols;
				} else {
					ulmapie->OFDMA_Symbol_offset = 0x00;	
					ulmapie->Subchannel_offset = ++Subchannel_offset;
					OFDMA_Symbol_offset = 0;
				}				
				UlMapIeNum++;
			}
		}
		bssf = bssf->next;
	}
	
	//Create Data Grant IE
	bssf = BsSF_head;
	while (bssf) {
		if (bssf->Direction == UL && bssf->ScheduleType == 6)
			BsSearchSsInfo(bssf->MacAddr, -1)->UplinkBandwidth += bssf->MaxTrafficRate/8/200;
		if (bssf->Direction == UL && bssf->ScheduleType == 2)
			if (Scheduler::instance().clock() - bssf->LastPollingTime >= (double)bssf->PollingInterval/1000) {
				BsSearchSsInfo(bssf->MacAddr, -1)->UplinkBandwidth += bssf->MaxTrafficRate/8/200;
				bssf->BwReqSize = bssf->MaxTrafficRate/8/200;
				bssf->LastPollingTime = Scheduler::instance().clock();
			}
		bssf = bssf->next;
	}
	
	modulcoding = QPSK_12;
	BytePerSuchan = 6;
	UIUC = 1;
	ssinfo = BsSsInfo_head;
	while (ssinfo) {
		if (ssinfo->ModulCoding == modulcoding && ssinfo->UplinkBandwidth != 0 && UlAvailSymbol != 0){
			ulmapie = CreateUlMapIe();
			ulmapie->CID = ssinfo->BasicCID;
			ulmapie->UIUC = UIUC;
			ulmapie->Duration = ssinfo->UplinkBandwidth/BytePerSuchan/3;
			if ((double)ssinfo->UplinkBandwidth/BytePerSuchan/BytePerSuchan/3 - ulmapie->Duration > 0)
				ulmapie->Duration += 1;
			if (UlAvailSymbol < ulmapie->Duration*3)
				ulmapie->Duration = UlAvailSymbol/3;
			if (ulmapie->Duration == 0)
				ulmapie->Duration = 1;
			UlAvailSymbol -= ulmapie->Duration*3;
			ulmapie->Repetition_Coding_Indication = 0;
			UlMapIeNum++;
		}
		ssinfo = ssinfo->next;
		if (!ssinfo) {
			switch (modulcoding) {
				case QPSK_12: ssinfo = BsSsInfo_head; modulcoding = QPSK_34; BytePerSuchan = 9; UIUC = 2; break;
				case QPSK_34: ssinfo = BsSsInfo_head; modulcoding = QAM16_12; BytePerSuchan = 12; UIUC = 3; break;
				case QAM16_12:ssinfo = BsSsInfo_head; modulcoding = QAM16_34; BytePerSuchan = 18; UIUC = 4; break;
				case QAM16_34:ssinfo = BsSsInfo_head; modulcoding = QAM64_23; BytePerSuchan = 24; UIUC = 5; break;
				case QAM64_23:ssinfo = BsSsInfo_head; modulcoding = QAM64_34; BytePerSuchan = 27; UIUC = 6; break;
				case QAM64_34:break;	
			}
		}
	}
	
	ssinfo = BsSsInfo_head;
	while (ssinfo) {
		ssinfo->PollingStatus = false;
		ssinfo->DownlinkBandwidth = 0;
		ssinfo->UplinkBandwidth = 0;
		ssinfo = ssinfo->next;
	}
	//printf("[2] Dl %d Ul %d\n",DlAvailSymbol,UlAvailSymbol);
	sendDLMAP();
	sendULMAP();
	if (FrameNumber_Counter%100 == 0) {
		DCD_Counter++;
		sendDCD();
		UCD_Counter++;
		sendUCD();
	}
	dlmapTimer.start(2*SymbolLength);
	return;
}

void
Mac802_16::BsScheduler()
{
	DlMapIe *dlmapie = NULL;
	dlmapie = DlMapIe_head;
	while (dlmapie) {
		if (BsSearchSsInfo(-1, dlmapie->CID)) {
			int Bandwidth = 0;
			int TrafficPriority = 0;
			switch (BsSearchSsInfo(-1, dlmapie->CID)->ModulCoding) {
				case QPSK_12: Bandwidth = dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*6*2; break;
				case QPSK_34: Bandwidth = dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*9*2; break;
				case QAM16_12:Bandwidth = dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*12*2; break;
				case QAM16_34:Bandwidth = dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*18*2; break;
				case QAM64_23:Bandwidth = dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*24*2; break;
				case QAM64_34:Bandwidth = dlmapie->No_OFDMA_Symbols*dlmapie->No_Subchannels*27*2; break;	
			}
			BsServiceFlow *bssf = NULL;
			bssf = BsSF_head;
			while (bssf) {
				if (bssf->Direction == DL && bssf->Status == Active && bssf->ScheduleType == 6 && BsSearchSsInfo(bssf->MacAddr, -1)->BasicCID == dlmapie->CID) {
					//printf("Downlink Bandwidth %d CID %d UGS SFID=%d req size %d[%d]\n",Bandwidth,dlmapie->CID,bssf->SFID,bssf->SduQueue.byteLength(),bssf->SduQueue.count(bssf->SduQueue.byteLength()));
					BsFragOrPack(bssf, bssf->SduQueue.count(bssf->SduQueue.byteLength()));
					Bandwidth -= bssf->PduQueue.byteLength();
					//printf("\tbs SduQ=%d\tPduQ=%d\n",bssf->SduQueue.byteLength(),bssf->PduQueue.byteLength());
				}
				if (bssf->Direction == DL && bssf->Status == Active && bssf->BwReqSize != 0 && bssf->ScheduleType >= 3 && bssf->ScheduleType <= 5 && BsSearchSsInfo(bssf->MacAddr, -1)->BasicCID == dlmapie->CID) {
					//printf("Downlink Bandwidth %d CID %d Polling SFID=%d req size %d[%d]\n",Bandwidth,dlmapie->CID,bssf->SFID,bssf->BwReqSize,bssf->SduQueue.count(bssf->BwReqSize));
					BsFragOrPack(bssf, bssf->SduQueue.count(bssf->BwReqSize));
					bssf->BwReqSize = 0;
					Bandwidth -= bssf->PduQueue.byteLength();
					//printf("\tbs SduQ=%d\tPduQ=%d\n",bssf->SduQueue.byteLength(),bssf->PduQueue.byteLength());
				}
				if (bssf->Direction == DL && bssf->Status == Active && bssf->ScheduleType == 2 && BsSearchSsInfo(bssf->MacAddr, -1)->BasicCID == dlmapie->CID) {
					TrafficPriority += bssf->TrafficPriority;
				}
				bssf = bssf->next;
			}
			if (Bandwidth > 0 && TrafficPriority > 0) {
				bssf = BsSF_head;
				while (bssf) {
					if (bssf->Direction == DL && bssf->Status == Active && bssf->ScheduleType == 2 && BsSearchSsInfo(bssf->MacAddr, -1)->BasicCID == dlmapie->CID) {
						//printf("Downlink Bandwidth %d CID %d BE SFID=%d req size %d[%d]\n",Bandwidth,dlmapie->CID,bssf->SFID,Bandwidth*bssf->TrafficPriority/TrafficPriority,bssf->SduQueue.count(Bandwidth*bssf->TrafficPriority/TrafficPriority));
						BsFragOrPack(bssf, bssf->SduQueue.count(Bandwidth*bssf->TrafficPriority/TrafficPriority));
						//printf("\tbs SduQ=%d\tPduQ=%d\n",bssf->SduQueue.byteLength(),bssf->PduQueue.byteLength());
					}
					bssf = bssf->next;
				}
			}
		}
		dlmapie = dlmapie->next;
	}
	BsBurstTransmit();
	return;
}

void
Mac802_16::SsScheduler()
{
	UlMapIe *ulmapie;
	if (!RangingStatus) {
		//Initial Ranging IE
		ulmapie = UlMapIe_head;
		while (ulmapie) {
			if (ulmapie->UIUC == 0xC && ulmapie->CID == 0xFFFF)
				sendRNGREQ(ulmapie);
			ulmapie = ulmapie->next;
		}
	} else {
		//Data Grant IE
		ulmapie = UlMapIe_head;
		while (ulmapie) {
			if (ulmapie->UIUC != 0xC && ulmapie->CID == BasicCID) {
				int Bandwidth = 0;
				int TrafficPriority = 0;
				switch (ModulCoding) {
					case QPSK_12: Bandwidth = ulmapie->Duration*6*3; break;
					case QPSK_34: Bandwidth = ulmapie->Duration*9*3; break;
					case QAM16_12:Bandwidth = ulmapie->Duration*12*3; break;
					case QAM16_34:Bandwidth = ulmapie->Duration*18*3; break;
					case QAM64_23:Bandwidth = ulmapie->Duration*24*3; break;
					case QAM64_34:Bandwidth = ulmapie->Duration*27*3; break;	
				}
				SsServiceFlow *sssf;
				sssf = SsSF_head;
				while (sssf) {
					if (sssf->Status == Active && sssf->ScheduleType == 6) {
						//printf("Uplink Bandwidth %d UGS SFID=%d req size %d[%d]\n",Bandwidth,sssf->SFID,sssf->SduQueue.byteLength(),sssf->SduQueue.count(sssf->SduQueue.byteLength()));
						SsFragOrPack(sssf, sssf->SduQueue.count(sssf->SduQueue.byteLength()));
						Bandwidth -= sssf->PduQueue.byteLength();
						//printf("\tss SduQ=%d\tPduQ=%d\n",sssf->SduQueue.byteLength(),sssf->PduQueue.byteLength());
					}
					if (sssf->Status == Active && sssf->BwReqSize != 0 && sssf->ScheduleType >= 3 && sssf->ScheduleType <= 5 ) {
						//printf("Uplink Bandwidth %d Polling SFID=%d req size %d[%d]\n",Bandwidth,sssf->SFID,sssf->BwReqSize,sssf->SduQueue.count(sssf->BwReqSize));
						SsFragOrPack(sssf, sssf->SduQueue.count(sssf->BwReqSize));
						sssf->BwReqSize = 0;
						Bandwidth -= sssf->PduQueue.byteLength();
						//printf("\tss SduQ=%d\tPduQ=%d\n",sssf->SduQueue.byteLength(),sssf->PduQueue.byteLength());
					}
					if (sssf->Status == Active && sssf->ScheduleType == 2) {
						TrafficPriority += sssf->TrafficPriority;
					}
					sssf = sssf->next;
				}
				if (Bandwidth > 0 && TrafficPriority > 0) {
					sssf = SsSF_head;
					while (sssf) {
						if (sssf->Status == Active && sssf->ScheduleType == 2) {
							//printf("Uplink Bandwidth %d BE SFID=%d req size %d[%d]\n",Bandwidth,sssf->SFID,Bandwidth*sssf->TrafficPriority/TrafficPriority,sssf->SduQueue.count(Bandwidth*sssf->TrafficPriority/TrafficPriority));
							SsFragOrPack(sssf, sssf->SduQueue.count(Bandwidth*sssf->TrafficPriority/TrafficPriority));
							//printf("\tss SduQ=%d\tPduQ=%d\n",sssf->SduQueue.byteLength(),sssf->PduQueue.byteLength());
						}
						sssf = sssf->next;
					}
				}
			}
			ulmapie = ulmapie->next;
		}
		
		//BW Request IE
		ulmapie = UlMapIe_head;
		while (ulmapie) {
			if (ulmapie->UIUC == 0xC && ulmapie->CID == BasicCID)
				sendBWREQ(ulmapie);
			ulmapie = ulmapie->next;
		}
	}
	SsBurstTransmit();
	return;
}

void 
Mac802_16::BsFragOrPack(BsServiceFlow *BsSF, int Size)
{	
	//printf("[1]BS SDU num=%d PDU num=%d\n",BsSF->SduQueue.length(),BsSF->PduQueue.length());
	static int32_t tmp_size = PDUSIZE;	
	Packet* frag;
	Packet* sdu;
BsStart:	
	while (Size) {				
		sdu = BsSF->SduQueue.deque();		
		struct hdr_cmn* sdu_hdr = HDR_CMN(sdu);
	    struct hdr_mac802_16* sdu_mac16 = HDR_MAC802_16(sdu);	
		
		if (sdu_hdr->size() <= tmp_size) {
			//need to pack
			sdu_mac16->generic_hdr.Type = 0x01;	
			sdu->type_ = 1;	
			struct subhdr_pack* psh = SUBHDR_Pack(sdu);
			//printf("Add packing subheader FSN:%d\n",psh->FSN);
			psh->FSN = (psh->FSN + 1)%8;				
			if (sdu_hdr->size() < tmp_size) {
				//Add PACKing subheader; add SDU or SDU fragment					
				psh->FC = 0x0;
				psh->Length = sdu_hdr->size();
				tmp_size -= sdu_hdr->size();
				//printf("Need to Pack:\n");	
				Packet::free(sdu);
			} else {
				//add PACKing subheader; add fragment					
				psh->FC = 0x2;
				frag = sdu;   
				BsSF->PduQueue.enque(sdu);	
				//printf("[1]Complete a MPDU\nPacking with fragmetation; The remaining fragment is stored in queue\n");					
				tmp_size = PDUSIZE;								
			}
			Size--;
			goto BsStart;				
		} else {
			//Need fragmetation
        	sdu_mac16->generic_hdr.Type = 0x04; 
			sdu->type_ = 2;		
			if (BsSF->FragQueue.length() != 0) { 		
BsFragQueue:	
				//Fragment-queue is not empty;
				Packet::free(sdu);				
				frag = BsSF->FragQueue.deque();
				struct hdr_cmn* frag_hdr = HDR_CMN(frag);
				struct subhdr_frag* fsh = SUBHDR_Frag(frag);				
				//printf("Add fragmentation subheader FSN:%d\n",fsh->FSN);
				fsh->FSN = (fsh->FSN + 1)%8;				
				if (frag_hdr->size() <= tmp_size) {
					//Add SDU to payload
					//printf("Add SDU to payload\nLast Fragment\n");						
					if (frag_hdr->size() < tmp_size) {
						// last fragment  need pack
						fsh->FC = 0x1;
						tmp_size -= frag_hdr->size();
						Packet::free(frag);
					} else {
						// last fragment
						fsh->FC = 0x1;
						// printf("Complete a MPDU\n\n");							 
						tmp_size = PDUSIZE;	
						BsSF->PduQueue.enque(frag);					
					}	
				} else {
					// continue fragment	
					fsh->FC = 0x3;		
					//printf("Continue Fragment\n");
					int i = frag_hdr->size();
					frag_hdr->size() = tmp_size;
					BsSF->PduQueue.enque(frag->copy());
					frag_hdr->size() = i - tmp_size;
					BsSF->FragQueue.enque(frag);
					//printf("[2]Complete a MPDU\n The remaining fragment is stored in queue\n");					
					tmp_size = PDUSIZE;
					goto BsFragQueue;	
				}
			} else {
				//there're no fragments in fragment-queue
				struct hdr_cmn* sdu_hdr = HDR_CMN(sdu);
				struct subhdr_frag* fsh = SUBHDR_Frag(sdu);
				//printf("Add fragmentation subheader FSN:%d\n",fsh->FSN);
				fsh->FSN = (fsh->FSN + 1)%8;
				if (sdu_hdr->size() > tmp_size) {
					//need to fragmentation
					int i = sdu_hdr->size();
					sdu_hdr->size() = PDUSIZE;
					//printf("First Fragmentaton\n");					 					
					fsh->FC = 0x2;  //first fragment
					BsSF->PduQueue.enque(sdu->copy());
					sdu_hdr->size() = i - tmp_size;
					BsSF->FragQueue.enque(sdu);
					//printf("[3]Complete a MPDU\n The remaining fragment is stored in queue\n");					
					tmp_size = PDUSIZE;					
					goto BsFragQueue;
				}
			}
		}
		Size--;
	}
	//printf("[2]BS SDU num=%d PDU num=%d\n",BsSF->SduQueue.length(),BsSF->PduQueue.length());
	return;
}

void 
Mac802_16::SsFragOrPack(SsServiceFlow *SsSF, int Size)
{	
	//printf("[1]SS SDU num=%d PDU num=%d\n",SsSF->SduQueue.length(),SsSF->PduQueue.length());
	static int32_t tmp_size = PDUSIZE;	
	Packet* frag;
	Packet* sdu;
SsStart:	
	while (Size) {				
		sdu = SsSF->SduQueue.deque();		
		struct hdr_cmn* sdu_hdr = HDR_CMN(sdu);
	    struct hdr_mac802_16* sdu_mac16 = HDR_MAC802_16(sdu);		
		if (sdu_hdr->size() <= tmp_size) {
			//need to pack
			sdu_mac16->generic_hdr.Type = 0x01;	
			sdu->type_ = 1;	
			struct subhdr_pack* psh = SUBHDR_Pack(sdu);
			//printf("Add packing subheader FSN:%d\n",psh->FSN);
			psh->FSN = (psh->FSN + 1)%8;				
			if (sdu_hdr->size() < tmp_size) {
				//Add PACKing subheader; add SDU or SDU fragment					
				psh->FC = 0x0;
				psh->Length = sdu_hdr->size();
				tmp_size -= sdu_hdr->size();
				//printf("Need to Pack:\n");	
				Packet::free(sdu);
			} else {
				//add PACKing subheader; add fragment					
				psh->FC = 0x2;
				frag = sdu;
				SsSF->PduQueue.enque(sdu);	
				//printf("[1]Complete a MPDU\nPacking with fragmetation; The remaining fragment is stored in queue\n");					
				tmp_size = PDUSIZE;								
			}
			Size--;
			goto SsStart;				
		} else {
			//Need fragmetation
        	sdu_mac16->generic_hdr.Type = 0x04; 
			sdu->type_ = 2;		
			if (SsSF->FragQueue.length() != 0) { 		
SsFragQueue:	
				//Fragment-queue is not empty;
				Packet::free(sdu);		
				frag = SsSF->FragQueue.deque();
				struct hdr_cmn* frag_hdr = HDR_CMN(frag);
				struct subhdr_frag* fsh = SUBHDR_Frag(frag);				
				//printf("Add fragmentation subheader FSN:%d\n",fsh->FSN);
				fsh->FSN = (fsh->FSN + 1)%8;				
				if (frag_hdr->size() <= tmp_size) {
					//Add SDU to payload
					//printf("Add SDU to payload\nLast Fragment\n");						
					if (frag_hdr->size() < tmp_size) {
						// last fragment  need pack
						fsh->FC = 0x1;
						tmp_size -= frag_hdr->size();
						Packet::free(frag);
					} else {
						// last fragment
						fsh->FC = 0x1;
						// printf("Complete a MPDU\n\n");							 
						tmp_size = PDUSIZE;	
						SsSF->PduQueue.enque(frag);					
					}	
				} else {
					// continue fragment	
					fsh->FC = 0x3;		
					//printf("Continue Fragment\n");
					int i = frag_hdr->size();
					frag_hdr->size() = tmp_size;
					SsSF->PduQueue.enque(frag->copy());
					frag_hdr->size() = i - tmp_size;
					SsSF->FragQueue.enque(frag);
					//printf("[2]Complete a MPDU\n The remaining fragment is stored in queue\n");					
					tmp_size = PDUSIZE;
					goto SsFragQueue;	
				}
			} else {
				//there're no fragments in fragment-queue
				struct hdr_cmn* sdu_hdr = HDR_CMN(sdu);
				struct subhdr_frag* fsh = SUBHDR_Frag(sdu);
				//printf("Add fragmentation subheader FSN:%d\n",fsh->FSN);
				fsh->FSN = (fsh->FSN + 1)%8;
				if (sdu_hdr->size() > tmp_size) {
					//need to fragmentation
					int i = sdu_hdr->size();
					sdu_hdr->size() = PDUSIZE;
					//printf("First Fragmentaton\n");					 					
					fsh->FC = 0x2;  //first fragment
					SsSF->PduQueue.enque(sdu->copy());
					sdu_hdr->size() = i - tmp_size;
					SsSF->FragQueue.enque(sdu);
					//printf("[3]Complete a MPDU\n The remaining fragment is stored in queue\n");					
					tmp_size = PDUSIZE;					
					goto SsFragQueue;
				}
			}
		}
		Size--;
	}
	//printf("[2]SS SDU num=%d PDU num=%d\n",SsSF->SduQueue.length(),SsSF->PduQueue.length());*/
	return;
}

void
Mac802_16::BsBurstTransmit()
{
	BsServiceFlow *bssf = NULL;
	bssf = BsSF_head;
	while (bssf) {
		while (bssf->Direction == DL && bssf->PduQueue.length() != 0) {
			Packet *p = bssf->PduQueue.deque();
			struct hdr_cmn *hdr = HDR_CMN(p);
			struct hdr_mac *mac = HDR_MAC(p); 
			struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);

			mac->macDA() = bssf->MacAddr;
			mac->macSA() = BsID;
			mac->set(MF_DATA, NodeID);
			state(MAC_SEND);
	
			hdr->uid() = 0;
			hdr->size() += sizeof(GenericHdr);
			hdr->iface() = -2;
			hdr->error() = 0; 
			hdr->txtime() = SymbolLength;
	
			mac16->generic_hdr.HT = 0;
			mac16->generic_hdr.EC = 1;
			mac16->generic_hdr.Type = 1;
			mac16->generic_hdr.CI = 1; 
			mac16->generic_hdr.EKS = 1;
			mac16->generic_hdr.LEN = 3;
			mac16->generic_hdr.CID = bssf->TransportCID;
			mac16->generic_hdr.HCS = 0;
			
			//printf("bs  send pkt\n");
			downtarget_->recv(p, this);
			bssf->QueueTime = Scheduler::instance().clock();			
		}		
		//printf("bs  SFID=%d\tSduQ=%d\tPduQ=%d\tFragQ=%d\n",bssf->SFID,bssf->SduQueue.length(),bssf->PduQueue.length(),bssf->FragQueue.length());
		bssf = bssf->next;
	}
	return;
}

void
Mac802_16::SsBurstTransmit()
{
	SsServiceFlow *sssf = NULL;
	sssf = SsSF_head;
	while (sssf) {
		while (sssf->PduQueue.length() != 0) {
			Packet *p = sssf->PduQueue.deque();
			struct hdr_cmn *hdr = HDR_CMN(p);
			struct hdr_mac *mac = HDR_MAC(p); 
			struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);

			mac->macDA() = BsID;
			mac->macSA() = NodeID;
			mac->set(MF_DATA, NodeID);
			state(MAC_SEND);
	
			hdr->uid() = 0;
			hdr->size() += sizeof(GenericHdr);
			hdr->iface() = -2;
			hdr->error() = 0; 
			hdr->txtime() = SymbolLength;
	
			mac16->generic_hdr.HT = 0;
			mac16->generic_hdr.EC = 1;
			mac16->generic_hdr.Type = 1;
			mac16->generic_hdr.CI = 1; 
			mac16->generic_hdr.EKS = 1;
			mac16->generic_hdr.LEN = 3;
			mac16->generic_hdr.CID = sssf->TransportCID;
			mac16->generic_hdr.HCS = 0;
			
			//printf("  ss%d sendpkt\n",NodeID);
			downtarget_->recv(p, this);
			sssf->QueueTime = Scheduler::instance().clock();
		}	
		//printf("ss%d SFID=%d\tSduQ=%d\tPduQ=%d\tFragQ=%d\n",NodeID,sssf->SFID,sssf->SduQueue.length(),sssf->PduQueue.length(),sssf->FragQueue.length());
		sssf = sssf->next;
	}
	return;
}

BsSsInfo*
Mac802_16::BsSearchSsInfo(int MacAddr, int CID)
{
	BsSsInfo *ssinfo = NULL;
	ssinfo = BsSsInfo_head;

	if (CID != -1) {
		while (ssinfo) {
			if (ssinfo->BasicCID == CID || ssinfo->PrimaryCID == CID)
				return ssinfo; 
			else
				ssinfo = ssinfo->next;
		}
	} else {
		while (ssinfo) {
			if (ssinfo->MacAddr == MacAddr)
				return ssinfo; 
			else
				ssinfo = ssinfo->next;
		}		
	}
	return NULL;
}

BsSsInfo*
Mac802_16::BsCreateSsInfo(Packet *p)
{	
	struct hdr_mac *mac = HDR_MAC(p);
	RngReqMsg *rngreq = (RngReqMsg*) p->accessdata(); 
	printf("\tbs  create ss%d info\n", mac->macSA());
	
	BsSsInfo *ssinfo = new BsSsInfo;
	ssinfo->MacAddr = rngreq->SS_MAC_Address;
	ssinfo->BasicCID = BasicCID_Counter++;
	ssinfo->PrimaryCID = PrimaryCID_Counter++;
	
	switch (rngreq->Requested_Downlink_Burst_Profile - 1) {
		case 0:ssinfo->ModulCoding = QPSK_12; break;
		case 1:ssinfo->ModulCoding = QPSK_34; break;
		case 2:ssinfo->ModulCoding = QAM16_12; break;
		case 3:ssinfo->ModulCoding = QAM16_34; break;
		case 4:ssinfo->ModulCoding = QAM64_23; break;
		case 5:ssinfo->ModulCoding = QAM64_34; break;
	}

	if (!BsSsInfo_head) {
		BsSsInfo_head = BsSsInfo_tail = ssinfo;
		ssinfo->next = NULL;
	} else {
		BsSsInfo_tail->next = ssinfo;
		BsSsInfo_tail = ssinfo;
		ssinfo->next = NULL;
	}
	return ssinfo;
}

BsServiceFlow*
Mac802_16::BsSearchServiceFlow(int SrcMacAddr, int DstMacAddr, packet_t ptype, SfDirection Direction, int TransactionID, int CID)
{
	int ScheduleType;
	BsServiceFlow *bssf = NULL;
	bssf = BsSF_head;
	switch (ptype) {
		case PT_UGS: ScheduleType = 0x06; break;
		case PT_ertPS: ScheduleType = 0x05; break;
		case PT_rtPS: ScheduleType = 0x04; break;
		case PT_nrtPS: ScheduleType = 0x03; break;
		case PT_BE: ScheduleType = 0x02; break;
		default: break;
	}
	
	if (TransactionID != -1) {
		while (bssf) {
			if (bssf->TransactionID == TransactionID)
				return bssf;
			else
				bssf = bssf->next;
		}
	} else if (TransactionID == -1 && CID != -1) {
		while (bssf) {
			if (bssf->TransportCID == CID)
				return bssf;
			else
				bssf = bssf->next;
		}
	} else {
		while (bssf) {
			if (bssf->SrcMacAddr == SrcMacAddr && bssf->DstMacAddr == DstMacAddr && bssf->ScheduleType == ScheduleType && bssf->Direction == Direction)
				return bssf;
			else
				bssf = bssf->next;
		}
	}
	return NULL;
}

BsServiceFlow*
Mac802_16::BsCreateServiceFlow(Packet *p, SfDirection Direction)
{
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p);
	struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);	
	DsaReqMsg *dsareq = (DsaReqMsg*) p->accessdata();
	BsServiceFlow *bssf = new BsServiceFlow;

	if (!CallAdmissionControl(p)) {
		delete bssf;
		return NULL;
	}
	
	if (Direction == UL) {
		bssf->MacAddr = BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr;
		bssf->DstMacAddr = dsareq->Service_Flow_Parameters.Destination_MAC_Address.Value;
		bssf->SrcMacAddr = dsareq->Service_Flow_Parameters.Source_MAC_Address.Value;
		bssf->TransactionID = dsareq->Transaction_ID;
		bssf->TransportCID = TransportCID_Counter++;
		if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x06) {	
			bssf->SFID = (ServiceFlowID_Counter0++)*10;
			bssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			bssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			bssf->ToleratedJitter = dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value;
			bssf->MaxLatency = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			bssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tbs  create ss%d UL UGS SF sfid=%d\n",BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr,bssf->SFID);	
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x05) { 	
			bssf->SFID = (ServiceFlowID_Counter1++)*10 + 1;
			bssf->PollingInterval = dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value;
			bssf->LastPollingTime = Scheduler::instance().clock();
			bssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			bssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			bssf->ToleratedJitter = dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value;
			bssf->MaxLatency = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			bssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tbs  create ss%d UL ertPS SF sfid=%d\n",BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr,bssf->SFID);
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x04) { 	
			bssf->SFID = (ServiceFlowID_Counter2++)*10 + 2;
			bssf->PollingInterval = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			bssf->LastPollingTime = Scheduler::instance().clock();
			bssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			bssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			bssf->MaxLatency = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			bssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tbs  create ss%d UL rtPS SF sfid=%d\n",BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr,bssf->SFID);
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x03) { 	
			bssf->SFID = (ServiceFlowID_Counter3++)*10 + 3;
			bssf->PollingInterval = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			bssf->LastPollingTime = Scheduler::instance().clock();
			bssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			bssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			bssf->TrafficPriority = dsareq->Service_Flow_Parameters.Traffic_Priority.Value;
			bssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tbs  create ss%d UL nrtPS SF sfid=%d\n",BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr,bssf->SFID);
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x02) {	
			bssf->SFID = (ServiceFlowID_Counter4++)*10 + 4;
			bssf->PollingInterval = 1000;
			bssf->LastPollingTime = Scheduler::instance().clock();
			bssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			bssf->TrafficPriority = dsareq->Service_Flow_Parameters.Traffic_Priority.Value;
			bssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tbs  create ss%d UL BE SF sfid=%d\n",BsSearchSsInfo(-1, mac16->generic_hdr.CID)->MacAddr,bssf->SFID);
		}	
		bssf->ScheduleType = dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value;
		bssf->Status = Active;	
		bssf->Direction = UL;	
	} else {	
		bssf->MacAddr = mac->macDA();//hdr->next_hop();
		bssf->SrcMacAddr = mac->macSA();
		bssf->DstMacAddr = mac->macDA();
		bssf->TransactionID = TransactionID_Counter++;
		bssf->TransportCID = TransportCID_Counter++;
		if (hdr->ptype() == PT_UGS) {// || dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x06) {
			bssf->SFID = (ServiceFlowID_Counter5++)*10 + 5;
			bssf->ScheduleType = 6;
			bssf->MaxTrafficRate = 64000; //64Kbps
			bssf->MinTrafficRate = 64000; //64Kbps
			bssf->ToleratedJitter = 10;
			bssf->MaxLatency = 50;
			bssf->ReqTxPolicy = 0;
			printf("\tbs  create ss%d DL UGS SF sfid=%d\n",mac->macDA(),bssf->SFID);	
		} else if (hdr->ptype() == PT_ertPS) {// || dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x05) {
			bssf->SFID = (ServiceFlowID_Counter6++)*10 + 6;
			bssf->ScheduleType = 5;
			bssf->MaxTrafficRate = 64000; //64Kbps
			bssf->MinTrafficRate = 8000; //8Kbps
			bssf->ToleratedJitter = 30;
			bssf->MaxLatency = 50;
			bssf->ReqTxPolicy = 0;
			printf("\tbs  create ss%d DL ertPS SF sfid=%d\n",mac->macDA(),bssf->SFID);	
		} else if (hdr->ptype() == PT_rtPS) {// || dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x04) {
			bssf->SFID = (ServiceFlowID_Counter7++)*10 + 7;
			bssf->ScheduleType = 4;
			bssf->MaxTrafficRate = 1024000; //1Mbps
			bssf->MinTrafficRate = 512000; //512Kbps
			bssf->MaxLatency = 50;
			bssf->ReqTxPolicy = 0;
			printf("\tbs  create ss%d DL rtPS SF sfid=%d\n",mac->macDA(),bssf->SFID);	
		} else if (hdr->ptype() == PT_nrtPS) {// || dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x03) {
			bssf->SFID = (ServiceFlowID_Counter8++)*10 + 8;
			bssf->ScheduleType = 3;
			bssf->MaxTrafficRate = 1024000; //1Mbps
			bssf->MinTrafficRate = 512000; //512Kbps
			bssf->MaxLatency = 500;
			bssf->TrafficPriority = 7;
			bssf->ReqTxPolicy = 0;
			printf("\tbs  create ss%d DL nrtPS SF sfid=%d\n",mac->macDA(),bssf->SFID);	
		} else if (hdr->ptype() == PT_BE) {// || dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x02) {
			bssf->SFID = (ServiceFlowID_Counter9++)*10 + 9;
			bssf->ScheduleType = 2;
			bssf->MaxTrafficRate = 1024000; //1Mbps
			bssf->MaxLatency = 1000;
			bssf->TrafficPriority = 7;
			bssf->ReqTxPolicy = 0;
			printf("\tbs  create ss%d DL BE SF sfid=%d\n",mac->macDA(),bssf->SFID);	
		}
		bssf->Status = Provisioned;
		bssf->Direction = DL;	
	}
	printf("\tMacAddr %d SrcMacAddr %d DstMacAddr %d\n",bssf->MacAddr,bssf->SrcMacAddr,bssf->DstMacAddr);
	
	if (!BsSF_head) {
		BsSF_head = BsSF_tail = bssf;
		bssf->next = NULL;
	} else {
		BsSF_tail->next = bssf;
		BsSF_tail = bssf;
		bssf->next = NULL;
	}
	return bssf;
}

SsServiceFlow*
Mac802_16::SsSearchServiceFlow(int DstMacAddr, packet_t ptype, SfDirection Direction, int TransactionID, int CID)
{	
	int ScheduleType;
	SsServiceFlow *sssf = NULL;
	sssf = SsSF_head;
	
	switch (ptype) {
		case PT_UGS: ScheduleType = 0x06; break;
		case PT_ertPS: ScheduleType = 0x05; break;
		case PT_rtPS: ScheduleType = 0x04; break;
		case PT_nrtPS: ScheduleType = 0x03; break;
		case PT_BE: ScheduleType = 0x02; break;
		default: break;
	}
	
	if (TransactionID != -1) {
		while (sssf) {
			if (sssf->TransactionID == TransactionID)
				return sssf;
			else
				sssf = sssf->next;
		}
	} else if (TransactionID == -1 && CID != -1) {
		while (sssf) {
			if (sssf->TransportCID == CID)
				return sssf;
			else
				sssf = sssf->next;
		}
	} else {
		while (sssf) {
			if (sssf->DstMacAddr == DstMacAddr && sssf->ScheduleType == ScheduleType && sssf->Direction == Direction)
				return sssf;
			else
				sssf = sssf->next;
		}
	}
	return NULL;
}

SsServiceFlow*
Mac802_16::SsCreateServiceFlow(Packet* p, SfDirection Direction)
{	
	struct hdr_cmn *hdr = HDR_CMN(p);
	struct hdr_mac *mac = HDR_MAC(p); 
	//struct hdr_mac802_16 *mac16 = HDR_MAC802_16(p);	
	DsaReqMsg *dsareq = (DsaReqMsg*) p->accessdata();
	SsServiceFlow *sssf = new SsServiceFlow;
	
	if (Direction == UL) {
		sssf->MacAddr = NodeID;
		sssf->DstMacAddr = mac->macDA();
		sssf->SrcMacAddr = mac->macSA();
		sssf->TransactionID = TransactionID_Counter++; //0x0000 ~ 0x7FFFF
		sssf->TransportCID = -1;
		sssf->SFID = -1;
		if (hdr->ptype() == PT_UGS) {
			printf("\tss%d create UL UGS SF\n",NodeID);	
			sssf->ScheduleType = 6;
			sssf->MaxTrafficRate = 64000; //64Kbps
			sssf->MinTrafficRate = 64000; //64Kbps
			sssf->ToleratedJitter = 10;
			sssf->MaxLatency = 50;
			sssf->ReqTxPolicy = 0;
		} else if (hdr->ptype() == PT_ertPS) { 	
			printf("\tss%d create UL ertPS SF\n",NodeID);	
			sssf->ScheduleType = 5;
			sssf->MaxTrafficRate = 64000; //64Kbps
			sssf->MinTrafficRate = 8000; //8Kbps
			sssf->ToleratedJitter = 30;
			sssf->MaxLatency = 50;
			sssf->ReqTxPolicy = 0;
		} else if (hdr->ptype() == PT_rtPS) { 	
			printf("\tss%d create UL rtPS SF\n",NodeID);	
			sssf->ScheduleType = 4;
			sssf->MaxTrafficRate = 1024000; //1Mbps
			sssf->MinTrafficRate = 512000; //512Kbps
			sssf->MaxLatency = 50;
			sssf->ReqTxPolicy = 0;
		} else if (hdr->ptype() == PT_nrtPS) { 	
			printf("\tss%d create UL nrtPS SF\n",NodeID);	
			sssf->ScheduleType = 3;
			sssf->MaxTrafficRate = 1024000; //1Mbps
			sssf->MinTrafficRate = 512000; //512Kbps
			sssf->MaxLatency = 500;
			sssf->TrafficPriority = 7;
			sssf->ReqTxPolicy = 0;
		} else if (hdr->ptype() == PT_BE) {	
			printf("\tss%d create UL BE SF\n",NodeID);	
			sssf->ScheduleType = 2;
			sssf->MaxTrafficRate = 1024000; //1Mbps
			sssf->TrafficPriority = 7;
			sssf->ReqTxPolicy = 0;
		}
		sssf->Status = Provisioned;
		sssf->Direction = UL;
	} else {
		sssf->MacAddr = BsID;
		sssf->DstMacAddr = dsareq->Service_Flow_Parameters.Destination_MAC_Address.Value;
		sssf->SrcMacAddr = dsareq->Service_Flow_Parameters.Source_MAC_Address.Value;
		sssf->TransactionID = dsareq->Transaction_ID;
		sssf->TransportCID =  dsareq->Service_Flow_Parameters.CID.Value;
		sssf->SFID = dsareq->Service_Flow_Parameters.SFID.Value;
		sssf->ScheduleType = dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value;
		if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x06) {	
			sssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			sssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			sssf->ToleratedJitter = dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value;
			sssf->MaxLatency = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			sssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tss%d  create DL UGS SF\n",NodeID);	
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x05) { 	
			sssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			sssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			sssf->ToleratedJitter = dsareq->Service_Flow_Parameters.Tolerated_Jitter.Value;
			sssf->MaxLatency = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			sssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tss%d  create DL ertPS SF\n",NodeID);
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x04) { 	
			sssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			sssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			sssf->MaxLatency = dsareq->Service_Flow_Parameters.Maximum_Latency.Value;
			sssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tss%d  create DL rtPS SF\n",NodeID);
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x03) { 	
			sssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			sssf->MinTrafficRate = dsareq->Service_Flow_Parameters.Minimum_Reserved_Traffic_Rate.Value;
			sssf->TrafficPriority = dsareq->Service_Flow_Parameters.Traffic_Priority.Value;
			sssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tss%d  create DL nrtPS SF\n",NodeID);
		} else if (dsareq->Service_Flow_Parameters.Service_Flow_Scheduling_Type.Value == 0x02) {	
			sssf->MaxTrafficRate = dsareq->Service_Flow_Parameters.Maximum_Sustained_Traffic_Rate.Value;
			sssf->TrafficPriority = dsareq->Service_Flow_Parameters.Traffic_Priority.Value;
			sssf->ReqTxPolicy = dsareq->Service_Flow_Parameters.Request_Transmission_Policy.Value;
			printf("\tss%d  create DL BE SF\n",NodeID);
		}
		sssf->Status = Active;			
		sssf->Direction = DL;	
	}
	
	if (!SsSF_head) {
		SsSF_head = SsSF_tail = sssf;
		sssf->next = NULL;
	} else {
		SsSF_tail->next = sssf;
		SsSF_tail = sssf;
		sssf->next = NULL;
	}
	return sssf;
}

DlMapIe*
Mac802_16::CreateDlMapIe()
{
	DlMapIe *tmp = new DlMapIe;
	if (!DlMapIe_head) {
		DlMapIe_head = DlMapIe_tail = tmp;
		tmp->next = NULL;
	} else {
		DlMapIe_tail->next = tmp;
		DlMapIe_tail = tmp;
		tmp->next = NULL;
	}
	return tmp;
}

UlMapIe*
Mac802_16::CreateUlMapIe()
{
	UlMapIe *tmp = new UlMapIe;
	if (!UlMapIe_head) {
		UlMapIe_head = UlMapIe_tail = tmp;
		tmp->next = NULL;
	} else {
		UlMapIe_tail->next = tmp;
		UlMapIe_tail = tmp;
		tmp->next = NULL;
	}
	return tmp;
}

void
Mac802_16::FrameTimerHandler()
{
	if (NodeID == BsID) {
		preambleTimer.start(SymbolLength);	
		
		UlMapIe *UlMapIe_tmp, *UlMapIe_backup;
		DlMapIe *DlMapIe_tmp, *DlMapIe_backup;
		
		UlMapIe_tmp = UlMapIe_head;
		while (UlMapIe_tmp) {
			UlMapIe_backup = UlMapIe_tmp->next;
			delete UlMapIe_tmp;
			UlMapIe_tmp = UlMapIe_backup;
		}
		UlMapIe_head = NULL;
		UlMapIe_tail = NULL;
		
		DlMapIe_tmp = DlMapIe_head;
		while (DlMapIe_tmp) {
			DlMapIe_backup = DlMapIe_tmp->next;
			delete DlMapIe_tmp;
			DlMapIe_tmp = DlMapIe_backup;
		}
		DlMapIe_head = NULL;
		DlMapIe_tail = NULL;
		
		if (FrameNumber_Counter%100 == 0) {
			UplinkBurstProfile *UlBurstProfile_tmp, *UlBurstProfile_backup;
			DownlinkBurstProfile *DlBurstProfile_tmp, *DlBurstProfile_backup;
		
			UlBurstProfile_tmp = UlBurstProfile_head;
			while (UlBurstProfile_tmp) {
				UlBurstProfile_backup = UlBurstProfile_tmp->next;
				delete UlBurstProfile_tmp;
				UlBurstProfile_tmp = UlBurstProfile_backup;
			}
			UlBurstProfile_head = NULL;
			UlBurstProfile_tail = NULL;
		
			DlBurstProfile_tmp = DlBurstProfile_head;
			while (DlBurstProfile_tmp) {
				DlBurstProfile_backup = DlBurstProfile_tmp->next;
				delete DlBurstProfile_tmp;
				DlBurstProfile_tmp = DlBurstProfile_backup;
			}
			DlBurstProfile_head = NULL;
			DlBurstProfile_tail = NULL;
		}
	}
}

void
Mac802_16::PreambleTimerHandler()
{
	if (NodeID == BsID) {	
		frameTimer.start(DlSymbolNum*SymbolLength + TtgLength + UlSymbolNum*SymbolLength + RtgLength);
		BandwidthManagement();
		FrameNumber_Counter++;
	}
}

void
Mac802_16::DownlinkTimerHandler(Packet *pkt)
{	
	//printf("downlink time %f\n",Scheduler::instance().clock());
	downtarget_->recv(pkt, this);
}

void
Mac802_16::UplinkTimerHandler(Packet *pkt)
{
	//printf("uplink time %f\n",Scheduler::instance().clock());
	downtarget_->recv(pkt, this);
}

int 
Mac802_16::command(int argc, const char*const* argv)
{
	if (argc == 3) 
	{
		if (strcmp(argv[1], "log-target") == 0) 
		{ 
			logtarget_ = (NsObject*) TclObject::lookup(argv[2]);
			if(logtarget_ == 0)
				return TCL_ERROR;
		}
	} 
	return Mac::command(argc, argv);
}

static class Mac802_16Class : public TclClass
{
public:
	Mac802_16Class() : TclClass("Mac/802_16") {}
	TclObject* create(int, const char*const*) 
	{
		return (new Mac802_16());
	}
} class_mac802_16;

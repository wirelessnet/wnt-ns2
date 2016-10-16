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
#ifndef ns_mac_80216_h
#define ns_mac_80216_h

#include "address.h"
#include "ip.h"
#include "packet-802_16.h"
#include "timer-802_16.h"
#include "marshall.h"

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "cmu-trace.h"
#include "basetrace.h"
#include "connector.h" 

enum ModulCodingType { QPSK_12 = 0, QPSK_34 = 1, QAM16_12 = 2, QAM16_34 = 3, QAM64_23 = 4, QAM64_34 = 5};
enum SfDirection { UL = 0, DL = 1};
enum SfStatus {Provisioned = 0, Admitted = 1, Active = 2};
//enum MacStatus {MSEND = 0, MIDLE = 1};

struct BsSsInfo {
	BsSsInfo(){next = NULL; BasicCID = -1; PrimaryCID = -1;}
	int MacAddr;
	int BasicCID;
	int PrimaryCID;
	int UplinkBandwidth;
	int DownlinkBandwidth;
	bool PollingStatus;
	ModulCodingType ModulCoding;
	BsSsInfo *next;
};

struct BsServiceFlow {
	BsServiceFlow(){next = NULL; TransportCID = -1;}
	int MacAddr;
	int TransactionID;
	int TransportCID;
	int DstMacAddr;
	int SrcMacAddr;
	int PollingInterval; //ms
	double LastPollingTime;
	int BwReqSize;
	int SFID; //Service Flow ID
	int ScheduleType; //Service_Flow_Scheduling_Type
	int MaxTrafficRate; //Maximum_Sustained_Traffic_Rate
	int MinTrafficRate; //Minimum_Reserved_Traffic_Rate
	int ToleratedJitter; //Tolerated_Jitter
	int MaxLatency; //Maximum_Latency
	int TrafficPriority; //Traffic_Priority
	int ReqTxPolicy; //Request_Transmission_Policy
	SfStatus Status; //QoS_Parameter_Set_Type
	double QueueTime;
	PacketQueue SduQueue;
	PacketQueue PduQueue;
	PacketQueue FragQueue;
	SfDirection Direction;	
	BsServiceFlow *next;
};

struct SsServiceFlow {
	SsServiceFlow(){next = NULL; TransportCID = -1;}
	int MacAddr;
	int DstMacAddr;
	int SrcMacAddr;
	int TransactionID;
	int TransportCID;
	int BwReqSize;
	int SFID; //Service Flow ID
	int ScheduleType; //Service_Flow_Scheduling_Type
	int MaxTrafficRate; //Maximum_Sustained_Traffic_Rate
	int MinTrafficRate; //Minimum_Reserved_Traffic_Rate	
	int ToleratedJitter; //Tolerated_Jitter
	int MaxLatency; //Maximum_Latency
	int TrafficPriority; //Traffic_Priority
	int ReqTxPolicy; //Request_Transmission_Policy
	SfStatus Status; //QoS_Parameter_Set_Type
	double QueueTime;
	PacketQueue SduQueue;
	PacketQueue PduQueue;
	PacketQueue FragQueue;
	SfDirection Direction;
	SsServiceFlow *next;
};

class Mac802_16 : public Mac
{		
	friend class FrameTimer;
	friend class PreambleTimer;
	friend class DownlinkTimer;
	friend class UplinkTimer;
	friend class UlMapTimer;
	friend class DlMapTimer;
	
	public:
		Mac802_16();
		void	sendUCD();	
		void	sendDCD();
		void	sendDLMAP();
		void	sendULMAP();
		void	sendRNGREQ(UlMapIe *IE);
		void	sendBWREQ(UlMapIe *IE);	
		void	sendRNGRSP(BsSsInfo *SsInfo);
		void	sendDSAREQ(SsServiceFlow *SsSF, BsServiceFlow *BsSF);
		void	sendDSARSP(SsServiceFlow *SsSF, BsServiceFlow *BsSF);
		void	sendDSAACK(Packet *pkt);
		void	sendDSXRVD(Packet *pkt);	
		bool	CallAdmissionControl(Packet *p);
		void	BandwidthManagement();
		void	BsScheduler();
		void	SsScheduler();
		void    BsFragOrPack(BsServiceFlow *BsSF, int Size);
		void 	SsFragOrPack(SsServiceFlow *SsSF, int Size);			
		void	BsBurstTransmit();
		void	SsBurstTransmit();
		BsSsInfo* BsSearchSsInfo(int MacAddr, int CID);
		BsSsInfo* BsCreateSsInfo(Packet *p);
		BsServiceFlow*	BsSearchServiceFlow(int SrcMacAddr, int DstMacAddr, packet_t ptype, SfDirection Direction, int TransactionID, int CID);
		BsServiceFlow*	BsCreateServiceFlow(Packet *p, SfDirection Direction);
		SsServiceFlow*	SsSearchServiceFlow(int DstMacAddr, packet_t ptype, SfDirection Direction, int TransactionID, int CID);
		SsServiceFlow*	SsCreateServiceFlow(Packet *p, SfDirection Direction);
		UlMapIe* CreateUlMapIe();
		DlMapIe* CreateDlMapIe();
		
	protected:
		int	NodeID;
		int	BasicCID;
		int	PrimaryCID;
		int	TransactionID_Counter;
		bool RangingStatus;
		ModulCodingType ModulCoding;
		int UcdConfigChangeCount;
		int DcdConfigChangeCount;
		UplinkBurstProfile UplinkBurstProfileTable;
		DownlinkBurstProfile DownlinkBurstProfileTable;
		PacketQueue	UgsQueue;
		PacketQueue	ertPsQueue;
		PacketQueue	rtPsQueue;
		PacketQueue	nrtPsQueue;
		PacketQueue	BeQueue;
		SsServiceFlow *SsSF_head;
		SsServiceFlow *SsSF_tail;
		FrameTimer	frameTimer;
		PreambleTimer preambleTimer;
		DownlinkTimer downlinkTimer;
		UplinkTimer uplinkTimer;
		UlMapTimer ulmapTimer;
		DlMapTimer dlmapTimer;
		void FrameTimerHandler();
		void PreambleTimerHandler();
		void DownlinkTimerHandler(Packet *pkt);
		void UplinkTimerHandler(Packet *pkt);
	private:
		MacState	rx_state_;	// incoming state (MAC_RECV or MAC_IDLE)
		MacState	tx_state_;	// outgoint state
		int			command(int argc, const char*const* argv);
		void 		recv(Packet *, Handler *);	
		NsObject*	logtarget_;
		inline void	mac_log(Packet *p) 
		{
            logtarget_->recv(p, (Handler*) 0);
        }		
};
#endif 

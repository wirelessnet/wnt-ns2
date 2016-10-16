/**************************************************************************************
* *Copyright (c) 2006 Regents of the University of Chang Gung 						*
* *All rights reserved.													*
 *																*
 * Redistribution and use in source and binary forms, with or without						*
 * modification, are permitted provided that the following conditions					*
 * are met:															*
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
#ifndef ns_packet_80216_h
#define ns_packet_80216_h
#include "packet.h"

struct GenericHdr {
	//table 5
    u_char    HT 		:1; 
	u_char    EC 		:1;
	u_char    Type 		:6; //table 6 
	u_char    Reserved1 :1;
	u_char    CI 		:1;
	u_char    EKS 		:2;
	u_char    Reserved2 :1;
	u_int16_t LEN 		:11; //bype
	u_int16_t CID 		:16;
	u_int8_t  HCS 		:8;
};

struct BwReqHdr {
	//table 7
	u_char    HT 		:1; 
	u_char    EC 		:1;
	u_char    Type 		:6;
	u_int32_t BR 		:19;
	u_int16_t CID 		:16;
	u_int8_t  HCS 		:8;
};

struct subhdr_pack {
	//table 11
	u_char    FC		:2;
	u_char    FSN		:3;
	u_int16_t Length	:11;	
	static int offset_;
	inline static int& offset() {return offset_;}
	inline static subhdr_pack* access(const Packet* p) {
		return (subhdr_pack*) p->access(offset_);
	}
};

struct subhdr_frag {
	//table 8
    u_char    FC		:2;
	u_char    FSN		:3;
	u_int16_t Length	:11;
	static int offset_;
	inline static int& offset() {return offset_;}
	inline static subhdr_frag* access(const Packet* p) {
		return (subhdr_frag*) p->access(offset_);
	}
};

struct hdr_mac802_16 {
	GenericHdr generic_hdr;
	BwReqHdr   bwreq_hdr;
	//PackSubhdr pack_subhdr;
	//FragSubhdr frag_subhdr;
    
	static int offset_;
	inline static int& offset() {return offset_;}
	inline static hdr_mac802_16* access(const Packet* p) {
		return (hdr_mac802_16*) p->access(offset_);
	}
};

struct TLV_LEN8 {
	TLV_LEN8(){Length = 0x18;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int8_t  Value									:8;
};

struct TLV_LEN9 {
	TLV_LEN9(){Length = 0x19;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int16_t Value									:9;
};

struct TLV_LEN13 {
	TLV_LEN13(){Length = 0x1D;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int16_t Value									:13;
};

struct TLV_LEN16 {
	TLV_LEN16(){Length = 0x20;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int16_t Value									:16;
};
	
struct TLV_LEN24 {
	TLV_LEN24(){Length = 0x28;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int32_t Value									:24;
};

struct TLV_LEN32 {
	TLV_LEN32(){Length = 0x30;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int32_t Value									:32;
};

struct TLV_LEN40 {
	TLV_LEN40(){Length = 0x38;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int64_t Value									:40;
};

struct TLV_LEN48 {
	TLV_LEN48(){Length = 0x40;}
	u_int8_t  Type									:8;
	u_int8_t  Length								:8;
	u_int64_t Value									:48;
};

struct UplinkBurstProfile {
	UplinkBurstProfile() {next = NULL;}
	//table 304
	u_int8_t  Type 									:8;
	u_int8_t  Length 								:8;
	u_char    Reserved 								:4;
	u_char    UIUC 									:4;
	//table 357
	TLV_LEN8  FEC_Code_and_modulation_type;
	TLV_LEN8  Ranging_data_ratio;
	TLV_LEN40 Normalized_CN_override; //table 334
	UplinkBurstProfile *next;
};

struct UcdMsg {
	//table 17
	u_int8_t  Management_Message_Type				:8;
	u_int8_t  Config_Change_Count					:8;
	u_int8_t  Rng_Backoff_Start						:8;
	u_int8_t  Rng_Backoff_End 						:8;
	u_int8_t  Req_Backoff_Start						:8;
	u_int8_t  Req_Backoff_End						:8;
	//table 349
	TLV_LEN8  Contention_based_reservation_timeout;
	TLV_LEN16 Bandwidth_request_opportunity_size;
	TLV_LEN16 Ranging_request_opportunity_size;
	TLV_LEN32 Frequency;
	//table 353
	TLV_LEN8  Initial_ranging_codes;
	TLV_LEN8  Periodic_ranging_codes;
	TLV_LEN8  Bandwidth_request_codes;
	TLV_LEN8  Periodic_ranging_backoff_start;
	TLV_LEN8  Periodic_ranging_backoff_end;
	TLV_LEN8  Start_of_ranging_codes_group;
	TLV_LEN8  Permutation_base;
	TLV_LEN9  UL_allocated_subchannels_bitmap;
	TLV_LEN13 Optional_permutation_UL_Allocated_subchannels_bitmap;
	TLV_LEN8  Band_AMC_Allocation_Threshold;
	TLV_LEN8  Band_AMC_Release_Threshold;
	TLV_LEN8  Band_AMC_Allocation_Timer;
	TLV_LEN8  Band_AMC_Release_Timer;
	TLV_LEN8  Band_Status_Reporting_MAX_Period;
	TLV_LEN8  Band_AMC_Retry_Timer;
	TLV_LEN8  Safety_Channel_Allocation_Threshold;
	TLV_LEN8  Safety_Channel_Release_Threshold;
	TLV_LEN8  Safety_Channel_Allocation_Timer;
	TLV_LEN8  Safety_Channel_Release_Timer;
	TLV_LEN8  Bin_Status_Reporting_MAX_Period;
	TLV_LEN8  Safety_Channel_Retry_Timer;
	TLV_LEN8  HARQ_ACK_delay_for_UL_burst;
	TLV_LEN8  CQICH_Band_AMCTransition_Delay;
};

struct DownlinkBurstProfile {
	DownlinkBurstProfile() {next = NULL;}
	//table 303
	u_int8_t  Type 									:8;
	u_int8_t  Length 								:8;
	u_char    Reserved 								:4;
	u_char    DIUC 									:4;
	//table 363
	TLV_LEN8  FEC_Code_type;
	TLV_LEN8  DIUC_Mandatory_exit_threshold; //figure 81
	TLV_LEN8  DIUC_Minimum_entry_threshold; //figure 81
	DownlinkBurstProfile *next;
};

struct DcdMsg {
	//table 15
	u_int8_t  Management_Message_Type				:8;
	u_int8_t  Reserved								:8;
	u_int8_t  Config_Change_Count					:8;
	//table 358
	TLV_LEN16 BS_EIRP;
	TLV_LEN8  Channel_Nr; //8.5.
	TLV_LEN8  TTG;
	TLV_LEN8  RTG;
	TLV_LEN16 RSS;
	TLV_LEN24 Channel_Switch_Frame_Number;
	TLV_LEN32 Frequency;
	TLV_LEN48 BS_ID;
	TLV_LEN8  Size_of_CQICH_ID_field;
	TLV_LEN8  HARQ_ACK_delay_for_DL_burst;
	TLV_LEN8  MAC_version; //11.1.3
};

struct DlMapIe {
	DlMapIe(){next = NULL;}
	//table 275
	u_char    DIUC 									:4;
	u_int8_t  N_CID	 								:8; //table 276
	u_int16_t CID 									:16;
	u_int8_t  OFDMA_Symbol_offset					:8;
	u_char    Subchannel_offset						:6;
	u_char    Boosting								:3;
	u_char    No_OFDMA_Symbols 						:7;
	u_char    No_Subchannels	 					:6;
	u_char    Repetition_Coding_Indication 			:2;
	DlMapIe *next;
};

struct DlMapMsg {
	//table 16
	u_int8_t  Management_Message_Type				:8;
	u_int8_t  DCD_Count								:8;
	u_int64_t BS_ID									:48;
	u_int8_t  No_OFDMA_Symbols	                    :8;
	//table 273
	u_int8_t  Frame_duration_code					:8;
	u_int32_t Frame_number							:24;
};

struct UlMapIe {
	UlMapIe(){next = NULL;}
	//table 287
	u_int16_t CID 									:16;
	u_char    UIUC	 								:4; //table 288
	u_int8_t  OFDMA_Symbol_offset					:8 ;
	u_char    Subchannel_offset						:7;
	u_char    No_OFDMA_Symbols 						:7;
	u_char    No_Subchannels	 					:7;
	u_char    Ranging_Method 						:2 ;
	u_int64_t Duration	 							:10; //see 8.4.3.1
	u_char    Repetition_Coding_Indication 			:2;
	UlMapIe *next;
};

struct UlMapMsg {
	//table 18
	u_int8_t  Management_Message_Type				:8;
	u_int8_t  Reserved								:8;
	u_int8_t  UCD_Count								:8;
	u_int32_t Allocation_Start_Time					:32;
	u_int8_t  No_OFDMA_Symbols	                    :8;
};

struct RngReqMsg {
	//table 19
	u_int8_t  Management_Message_Type				:8;
	u_int8_t  Reserved								:8;
	//table 364
	u_int8_t  Requested_Downlink_Burst_Profile 		:8;
	u_int64_t SS_MAC_Address 						:48;
	u_int8_t  Ranging_Anomalies 					:8;
	u_int8_t  AAS_broadcast_capability 				:8;
};

struct RngRspMsg {
	//table 20
	u_int8_t  Management_Message_Type				:8;
	u_int8_t  Reserved								:8;
	//table 367
	u_int32_t Timing_Adjust 						:32;
	u_int8_t  Power_Level_Adjust 					:8;
	u_int32_t Offset_Frequency_Adjust 				:32;
	u_int8_t  Ranging_Status 						:8;
	u_int32_t Downlink_frequency_override 			:32;
	u_int8_t  Uplink_channel_ID_override 			:8;
	u_int16_t Downlink_Operational_Burst_Profile	:16;
	u_int64_t SS_MAC_Address 						:48;
	u_int16_t Basic_CID 							:16;
	u_int16_t Primary_Management_CID 				:16;
	u_int8_t  AAS_broadcast_permission 				:8;
};

struct Service_Flow {
	//table 383
	TLV_LEN32 SFID; 
	TLV_LEN16 CID;
	TLV_LEN8  QoS_Parameter_Set_Type; //table 385
	TLV_LEN8  Traffic_Priority; //0 (low) ~ 7 (high)
	TLV_LEN32 Maximum_Sustained_Traffic_Rate; //bit per second
	TLV_LEN32 Maximum_Traffic_Burst; //byte
	TLV_LEN32 Minimum_Reserved_Traffic_Rate; //bit per second
	TLV_LEN8  Service_Flow_Scheduling_Type; //11.13.11
	TLV_LEN32 Request_Transmission_Policy; //11.13.12
	TLV_LEN32 Tolerated_Jitter; //ms
	TLV_LEN32 Maximum_Latency; //ms
	TLV_LEN8  SDU_Indicator; //byte
	TLV_LEN8  SDU_Size; //byte
	TLV_LEN16 Target_SAID;
	TLV_LEN48 Destination_MAC_Address;
	TLV_LEN48 Source_MAC_Address;
};

struct DsaReqMsg {
	//table 38
	u_int8_t  Management_Message_Type				:8;
	u_int16_t Transaction_ID						:16;
	//table 383
	Service_Flow Service_Flow_Parameters;
};

struct DsaRspMsg {
	//table 39
	u_int8_t  Management_Message_Type				:8;
	u_int16_t Transaction_ID						:16;
	u_int8_t  Confirmation_Code						:8; //table 384
	//table 383
	Service_Flow Service_Flow_Parameters;
};

struct DsaAckMsg {
	//table 40
	u_int8_t  Management_Message_Type				:8;
	u_int16_t Transaction_ID						:16;
	u_int8_t  Confirmation_Code						:8; //table 384
};

struct DsxRvdMsg {
	//table 56
	u_int8_t  Management_Message_Type				:8;
	u_int16_t Transaction_ID						:16;
	u_int8_t  Confirmation_Code						:8; //table 384
};
#endif

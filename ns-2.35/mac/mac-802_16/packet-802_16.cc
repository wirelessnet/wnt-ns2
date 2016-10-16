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
#include "packet-802_16.h"

int hdr_mac802_16::offset_;
static class Mac802_16HeaderClass :public PacketHeaderClass {
	public:
		Mac802_16HeaderClass() : PacketHeaderClass("PacketHeader/802_16", sizeof(hdr_mac802_16)) {
			bind_offset(&hdr_mac802_16::offset_);
		}
} class_hdr_mac802_16;

int subhdr_pack::offset_;
static class PackSubHeaderClass :public PacketHeaderClass {
	public:
		PackSubHeaderClass() : PacketHeaderClass("PacketHeader/pack", sizeof(subhdr_pack)) {
			bind_offset(&subhdr_pack::offset_);
		}
} class_subhdr_pack;

int subhdr_frag::offset_;
static class FragSubHeaderClass :public PacketHeaderClass {
	public:
		FragSubHeaderClass() : PacketHeaderClass("PacketHeader/frag", sizeof(subhdr_frag)) {
			bind_offset(&subhdr_frag::offset_);
		}
} class_subhdr_frag;

/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * emuopl.cpp - Emulated OPL, by Simon Peter <dn.tlp@gmx.net>
 */

#include "emuopl.h"

CEmuopl::CEmuopl(int rate, bool bit16, bool usestereo)
  : use16bit(bit16), stereo(usestereo), oplRate(rate)
{
  YM3812Init(1, 3579545, rate);
}

CEmuopl::~CEmuopl()
{
  YM3812Shutdown();
}

void CEmuopl::update(short *buf, int samples)
{
	int i;

	if(use16bit) {
		YM3812UpdateOne(0,buf,samples);

		if(stereo)
			for(i=samples-1;i>=0;i--) {
				buf[i*2] = buf[i];
				buf[i*2+1] = buf[i];
			}
	} else {
		short *tempbuf = new short[stereo ? samples*2 : samples];
		int i;

		YM3812UpdateOne(0,tempbuf,samples);

		if(stereo)
			for(i=samples-1;i>=0;i--) {
				tempbuf[i*2] = tempbuf[i];
				tempbuf[i*2+1] = tempbuf[i];
			}

		for(i=0;i<(stereo ? samples*2 : samples);i++)
			((char *)buf)[i] = (tempbuf[i] >> 8) ^ 0x80;

		delete [] tempbuf;
	}
}

void CEmuopl::write(int reg, int val)
{
  YM3812Write(0,0,reg);
  YM3812Write(0,1,val);
}

void CEmuopl::init()
{
  YM3812ResetChip(0);
}

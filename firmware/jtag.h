/*
 *  tinyJTAG - A JTAG programmer for AVR microcontrollers
 *  Copyright (C) 2012 Louis Beaudoin
 *
 *  http://www.embedded-creations.com
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 *
 */

#ifndef _JTAG_H_
#define _JTAG_H_

#include <avr/io.h>
#include "types.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>


//#define JTAG_FLASH_BYTE_FUNCTIONS
//#define JTAG_PROG_FLASH_PAGE_FUNCTION
//#define JTAG_FULL_PAGE_FUNCTIONS

//#define EEPROMTEST

#define EEPROM __attribute__ ((section (".eeprom")))

#ifdef EEPROMTEST
#define JTAGCONST u_char EEPROM
#else
#define JTAGCONST prog_char
#endif

void JTAGBusReset(void);
void JTAGBusIdle(void);
void JTAGInit(void);

u_long JTAGReadDeviceID(void);
int AVRJTAGEnterProgrammingMode(void);
void AVRJTAGLeaveProgrammingMode(void);
void AVRJTAGEraseChip(void);

void AVRJTAGProgramFlashPage(u_char * buffer, u_long address, u_char length);
void AVRJTAGProgramLargeFlashPage(u_char * buffer, u_long address);
void AVRJTAGReadFlash(u_char * buffer, u_long address, u_int length);
void AVRJTAGReadEEPROM(u_char * buffer, u_int address, u_int length);
u_char AVRJTAGReadCalibrationByte(u_char address);
void AVRJTAGLoadFlashPage(u_char * buffer, u_long address, u_char length);
void AVRJTAGWriteFlashPage(u_long address);

u_char AVRJTAGReadSignature(u_char address);

void AVRJTAGReadLargeFlashPage(u_char * buffer, u_long address);
void AVRJTAGProgramEEPROMByte(u_char byte, u_int address);

u_char AVRJTAGReadFuses(void * parameter);
void AVRJTAGWriteCombFuseBits(u_char fusebyte, void * parameter);
void AVRJTAGWriteLockBits(u_char lockBits);

#define AVRJTAGReadFuseBits() AVRJTAGReadFuses(ReadFuseBitsCommand)
#define AVRJTAGReadExtFuseBits() AVRJTAGReadFuses(ReadExtFuseBitsCommand)
#define AVRJTAGReadLockBits() AVRJTAGReadFuses(ReadLockBitsCommand)
#define AVRJTAGReadHighFuseBits() AVRJTAGReadFuses(ReadHighFuseBitsCommand)

#define AVRJTAGWriteFuseBits(x) AVRJTAGWriteCombFuseBits(x, WriteFuseBits2);
#define AVRJTAGWriteHighFuseBits(x) AVRJTAGWriteCombFuseBits(x, WriteHighFuseBits2);
#define AVRJTAGWriteExtFuseBits(x) AVRJTAGWriteCombFuseBits(x, WriteExtendedFuseBits2);

extern JTAGCONST ReadFuseBitsCommand[];
extern JTAGCONST ReadExtFuseBitsCommand[];
extern JTAGCONST ReadLockBitsCommand[];
extern JTAGCONST ReadHighFuseBitsCommand[];

extern JTAGCONST WriteHighFuseBits2[];
extern JTAGCONST WriteLockBits2[];
extern JTAGCONST WriteExtendedFuseBits2[];
extern JTAGCONST WriteFuseBits2[];

u_char interpretSPICommand(u_char command[4]);

#endif

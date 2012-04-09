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

#include <avr/io.h>
//#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "jtag.h"
#include "stkparameters.h"
#include "buffuart.h"
#include "delay.h"
#include "avrdevicecodes.h"
#include "hardware.h"
#include "avrispinstructs.h"
#include <stdint.h>

#define NOP() asm volatile("nop")

extern void JTAGDRScan(u_int TDISeq, u_char length);

u_char newjtag;

void JTAGPulse(void)
{
    SET_JTAG_TCK();
    NOP();
    CLEAR_JTAG_TCK();

}

void JTAGShiftDRState(void)
{
    CLEAR_JTAG_TDI();
    SET_JTAG_TMS();
    JTAGPulse();
    
    CLEAR_JTAG_TMS();
    JTAGPulse();
    JTAGPulse();

}

void updateToShiftDR(void)
{
    CLEAR_JTAG_TDI();
    SET_JTAG_TMS();
    JTAGPulse();


    JTAGShiftDRState();
}    

void JTAGUpdateDRState(void)
{
    CLEAR_JTAG_TDI();
    SET_JTAG_TMS();
    JTAGPulse();

    CLEAR_JTAG_TMS();
    JTAGPulse();

}    

u_char interpretSPICommand(u_char command[4])
{
    uint16_t address = (command[1] << 8) | command[2];
    
    // read signature byte
    // read fuse/lock bits
    if(command[0] == AVRSPI_READ_FUSE_BITS_BYTE1)
    {
        if(command[1] == AVRSPI_READ_EXT_FUSE_BITS_BYTE2)
        {
            return AVRJTAGReadExtFuseBits();
        }
        else if(command[1] == AVRSPI_READ_FUSE_BITS_BYTE2)
        {
            return AVRJTAGReadFuseBits();
        }
    }
    // read lock bits / fuse high bits
    else if(command[0] == AVRSPI_READ_FUSE_HIGH_BITS_BYTE1)
    {
        if(command[1] == AVRSPI_READ_FUSE_HIGH_BITS_BYTE2)
        {
            return AVRJTAGReadHighFuseBits();
        }
        else if(command[1] == AVRSPI_READ_LOCK_BITS_BYTE2)
        {
            return AVRJTAGReadLockBits();
        }
    }
    // write lock / fuse bits / chip erase
    else if(command[0] == AVRSPI_WRITE_LOCK_BITS_BYTE1)
    {
        if((command[1] & AVRSPI_ERASE_CHIP_BYTE2_MASK) == AVRSPI_ERASE_CHIP_BYTE2_MATCH)
        {
            AVRJTAGEraseChip();
        }
		else if((command[1] & AVRSPI_WRITE_LOCK_BITS_BYTE2_MASK) == AVRSPI_WRITE_LOCK_BITS_BYTE2_MATCH)
		{
			AVRJTAGWriteLockBits(command[3]);
		}
		else if(command[1] == AVRSPI_WRITE_FUSE_BITS_BYTE2)
		{
			AVRJTAGWriteFuseBits(command[3]);
		}
		else if(command[1] == AVRSPI_WRITE_FUSE_HIGH_BITS_BYTE2)
		{
			AVRJTAGWriteHighFuseBits(command[3]);
		}
		else if(command[1] == AVRSPI_WRITE_EXT_FUSE_BITS_BYTE2)
		{
			AVRJTAGWriteExtFuseBits(command[3]);
		}
    }	
	else if(command[0] == AVRSPI_READ_SIGNATURE_BYTE1)
    {
        return AVRJTAGReadSignature(command[2]);
    }
	// read calibration byte
    else if(command[0] == AVRSPI_READ_CALIBRATION_BYTE1)
    {
		return AVRJTAGReadCalibrationByte(command[3]);
    }
    // Read EEPROM Memory
    else if(command[0] == AVRSPI_READ_EEPROM_BYTE1)
    {
        uint8_t tempBuffer[1];
		AVRJTAGReadEEPROM(tempBuffer, address, 1);
        return tempBuffer[0];
    }
    // Write EEPROM Memory
    else if(command[0] == AVRSPI_WRITE_EEPROM_BYTE1)
    {
        AVRJTAGProgramEEPROMByte(command[3], address);
    }
    // Read Program Memory (high or low)
    else if(command[0] == AVRSPI_READ_PROGMEM_LOW_BYTE1)
    {
        uint8_t tempBuffer[2];
        AVRJTAGReadFlash(tempBuffer, address, 1);
        return tempBuffer[0];
    }
    else if(command[0] == AVRSPI_READ_PROGMEM_HIGH_BYTE1)
    {
        uint8_t tempBuffer[2];
        AVRJTAGReadFlash(tempBuffer, address, 1);
        return tempBuffer[1];
    }

    // single writes to flash fail as it requires a write program operation after the load that never is sent from AVRDUDE
    // code below is untested
    #ifdef JTAG_FLASH_BYTE_FUNCTIONS
    else if(command[0] == AVRSPI_LOAD_PROGMEM_PAGE_HIGH_BYTE1)
    {
        uint16_t tempint = 0xFF00 | command[3];
        AVRJTAGLoadFlashPage((uint8_t *)&tempint, (uint8_t) address, 1);
    }
    else if(command[0] == AVRSPI_LOAD_PROGMEM_PAGE_LOW_BYTE1)
    {
        uint16_t tempint = (command[3] << 8) | 0xFF;
        AVRJTAGLoadFlashPage((uint8_t *)&tempint, (uint8_t) address, 1);
    }
    else if(command[0] == AVRSPI_WRITE_FLASH_PAGE_BYTE1)
    {
        AVRJTAGWriteFlashPage(address);
    }
#endif

	return 0;
}

u_int JTAGDRScanRead(u_int TDISeq, u_char length)
{
    u_int TDOSeq=0;
    u_long tdoval;
    
    // change to the Shift-DR state
    JTAGShiftDRState();
            
    tdoval = 1;
    
    while(length--)
    {
        if(!length)
            SET_JTAG_TMS();
            
            
        CLEAR_JTAG_TDI();

        if(TDISeq & 0x01)
            SET_JTAG_TDI();

        SET_JTAG_TCK();
        NOP();

        // read TDO
        if(READ_JTAG_TDO())
            TDOSeq |= tdoval;
        
        CLEAR_JTAG_TCK();

        tdoval <<= 1;
        TDISeq >>=1;
    }

    // change to the Update-DR state
    JTAGUpdateDRState();

    return TDOSeq;
}



#define JTAG_IR_SCAN        0x40
#define JTAG_DR_SCAN        0x80
#define JTAG_DRRTI_SCAN     0xC0
#define JTAG_DROUT_SCAN     0xA0
#define JTAG_SCAN_MASK      0xE0
#define JTAG_IR_SCAN_MASK   0x40
#define JTAG_DR_SCAN_MASK   0x80



#ifdef EEPROMTEST
u_int JTAGNewScan(void * command)
#else
u_int JTAGNewScan(PGM_P command)   
#endif 
{
    u_int TDOSeq=0;
    u_char i=0;
    u_int tdoval;
    u_int TDISeq;
    u_char length;
    u_char seqtype;

#ifdef EEPROMTEST
    while( eeprom_read_byte(command +i) != 0)
#else
    while( pgm_read_byte(command +i) != 0 )
#endif
    {
#ifdef EEPROMTEST
        seqtype = eeprom_read_byte(command +i) & JTAG_SCAN_MASK;
#else
        seqtype = pgm_read_byte(command +i) & JTAG_SCAN_MASK;
#endif
            
        CLEAR_JTAG_TDI();
        SET_JTAG_TMS();
        JTAGPulse();


        if(!(seqtype & JTAG_DR_SCAN_MASK))
        {
            JTAGPulse();

        }

        CLEAR_JTAG_TMS();
        JTAGPulse();

        JTAGPulse();


#ifdef EEPROMTEST
        length = (eeprom_read_byte(command +i) & ~JTAG_SCAN_MASK) + 1;
#else
        length = (pgm_read_byte(command +i) & ~JTAG_SCAN_MASK) + 1;
#endif
        
        // load the TDI buffer, and increment i to the next command
#ifdef EEPROMTEST
        eeprom_read_block(&TDISeq, command + i + 1, 2);
#else
        memcpy_P(&TDISeq, command + i + 1, 2);
#endif
        i+=2 + 1;
        
        TDOSeq=0;
        // shift in the command - leaving the Shift state on the last bit
        tdoval = 1;
        
        if(seqtype == JTAG_DROUT_SCAN)
            while(length--)
            {
                if(!length)
                    SET_JTAG_TMS();
                CLEAR_JTAG_TDI();

                if(TDISeq & 0x01)
                    SET_JTAG_TDI();

                SET_JTAG_TCK();
                NOP();

                // read TDO
                if(READ_JTAG_TDO())
                    TDOSeq |= tdoval;
                
                CLEAR_JTAG_TCK();

                tdoval <<= 1;
                TDISeq >>=1;
            }
        else
        {
            while(length--)
            {
                if(!length)
                    SET_JTAG_TMS();
                    
                CLEAR_JTAG_TDI();

                if(TDISeq & 0x01)
                    SET_JTAG_TDI();

                JTAGPulse();

                TDISeq >>=1;
            }
        }

        // change to the Update state
        CLEAR_JTAG_TDI();

        SET_JTAG_TMS();
        JTAGPulse();


        if( seqtype == JTAG_DRRTI_SCAN )
        {
            CLEAR_JTAG_TMS();
            JTAGPulse();
        }
    }

    return TDOSeq;
}
//#define ee_uchar u_char __attribute__ ((section (".eeprom")))

#ifdef EEPROMTEST
#define JTAGCONST u_char EEPROM
#else
#define JTAGCONST prog_char
#endif

// dummy variable to keep the JTAG constants out of the first EEPROM variable
// this may fix the bug of JTAG failing after connecting the programmer wrong
JTAGCONST dummy_eeprom[] = {0xFF};

/*ee_uchar ReadFuseBitsCommandEE[] = {    JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                    JTAG_DRRTI_SCAN | (15-1), 0x04, 0x23,
                                    JTAG_DRRTI_SCAN | (15-1), 0x00, 0x32,
                                    JTAG_DROUT_SCAN | (15-1), 0x00, 0x33,
                                    0x00 };*/

JTAGCONST ReadFuseBitsCommand[] = {    JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                    JTAG_DRRTI_SCAN | (15-1), 0x04, 0x23,
                                    JTAG_DRRTI_SCAN | (15-1), 0x00, 0x32,
                                    JTAG_DROUT_SCAN | (15-1), 0x00, 0x33,
                                    0x00 };



u_char AVRJTAGReadFuses(void * parameter)
{
    return (u_char) JTAGNewScan(parameter);
    
}



JTAGCONST ReadHighFuseBitsCommand[] = {JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                    JTAG_DRRTI_SCAN | (15-1), 0x04, 0x23,
                                    JTAG_DRRTI_SCAN | (15-1), 0x00, 0x3E,
                                    JTAG_DROUT_SCAN | (15-1), 0x00, 0x3F,
                                    0x00 };


JTAGCONST ReadLockBitsCommand[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x04, 0x23,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x36,
                                JTAG_DROUT_SCAN | (15-1), 0x07, 0x36,
                                0x00 };


JTAGCONST ReadExtFuseBitsCommand[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x04, 0x23,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x3A,
                                JTAG_DROUT_SCAN | (15-1), 0x00, 0x3B,
                                0x00 };


JTAGCONST EraseChipCommand[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x80, 0x23,
                                JTAG_DRRTI_SCAN | (15-1), 0x80, 0x31,
                                JTAG_DRRTI_SCAN | (15-1), 0x80, 0x33,
                                JTAG_DR_SCAN | (15-1), 0x80, 0x33,
                                0x00 };

void AVRJTAGEraseChip(void)
{
    JTAGNewScan(EraseChipCommand);   
}


JTAGCONST ProgFlashPageCommand1[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x10, 0x23,
                                        0x00 };

JTAGCONST ProgFlashPageCommand2[] = {   JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x77,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        0x00 };

JTAGCONST ProgFlashPageCommand3[] = {   JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x35,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        0x00 };


void AVRJTAGLoadFlashPage(u_char * buffer, u_long address, u_char length)
{
    u_char i;
    
    JTAGNewScan(ProgFlashPageCommand1);
    for(i=0; i<length; i++)
    {
        JTAGDRScan(0x0300 | (u_char)address, 15);
        JTAGDRScan(0x1300 | *buffer++, 15);
        JTAGDRScan(0x1700 | *buffer++, 15);
        JTAGNewScan(ProgFlashPageCommand2);
        address++;
    }
}

void AVRJTAGScan3ByteAddress(u_long address)
{
    JTAGDRScan(0x0B00 | (u_char)(address>>16), 15);
    JTAGDRScan(0x0700 | (u_char)(address>>8), 15);
    JTAGDRScan(0x0300 | (u_char)address, 15);
}

// word address, not byte address
void AVRJTAGWriteFlashPage(u_long address)
{
    JTAGNewScan(ProgFlashPageCommand1);
    AVRJTAGScan3ByteAddress(address);
    JTAGNewScan(ProgFlashPageCommand3);

    while(!(JTAGDRScanRead(0x3700, 15) & (1<<9)));
}

#ifdef JTAG_PROG_FLASH_PAGE_FUNCTION
void AVRJTAGProgramFlashPage(u_char * buffer, u_long address, u_char length)
{
    AVRJTAGLoadFlashPage(buffer, address, length);
    AVRJTAGWriteFlashPage(address);
}
#endif

#ifdef JTAG_FULL_PAGE_FUNCTIONS
JTAGCONST ProgLargeFlashPageCommand1[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x10, 0x23,
                                        0x00 };

JTAGCONST ProgLargeFlashPageCommand2[] = {   JTAG_IR_SCAN | (4-1), 0x06, 0x00,
                                        0x00 };

JTAGCONST ProgLargeFlashPageCommand3[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x35,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        0x00 };


void AVRJTAGProgramLargeFlashPage(u_char * buffer, u_long address)
{
    u_int length = flashpagesize * 8;
    u_char i=0;
    u_char temp=0;
    
    JTAGNewScan(ProgLargeFlashPageCommand1);
	AVRJTAGScan3ByteAddress(address);
    

    // PAGELOAD instruction, shift the bits
    JTAGNewScan(ProgLargeFlashPageCommand2);
    
    // navigate to Shift-DR
    JTAGShiftDRState();
    
    while(length--)
    {
        if(!length || (newjtag && i==1))
            SET_JTAG_TMS();

        if(i==0)
        {
            temp = *buffer++;
            i=8;
        }

        // set TDI
        CLEAR_JTAG_TDI();

        if(temp & 0x01)
        {
            // set TDI bit
            SET_JTAG_TDI();
        }

        // cycle TCK
        JTAGPulse();


        temp>>=1;
        i--;

        if(i==0 && length && newjtag)
        {
            // navigate through Update-DR back to Shift-DR
            updateToShiftDR();
        }
    }

    // change to the Update state
    JTAGUpdateDRState();
    
    JTAGNewScan(ProgLargeFlashPageCommand3);

    while(!(JTAGDRScanRead(0x3700, 15) & (1<<9)));
}
#endif


#ifdef JTAG_FULL_PAGE_FUNCTIONS
JTAGCONST ReadLargeFlashPageCommand1[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x02, 0x23,
                                        0x00 };

JTAGCONST ReadLargeFlashPageCommand2[] = {   JTAG_IR_SCAN | (4-1), 0x07, 0x00,
                                        0x00 };


void AVRJTAGReadLargeFlashPage(u_char * buffer, u_long address)
{
    u_int length = flashpagesize * 8;
    u_char i=0;
    u_char temp=0;
    
    JTAGNewScan(ReadLargeFlashPageCommand1);
	AVRJTAGScan3ByteAddress(address);
    

    // PAGELOAD instruction, shift the bits
    JTAGNewScan(ReadLargeFlashPageCommand2);
    
    // navigate to Shift-DR
    JTAGShiftDRState();
    
    CLEAR_JTAG_TDI();
        
    /*i=7;
    while(i--)
    {
        SET_JTAG_TCK();
        NOP();
        CLEAR_JTAG_TCK();
        NOP();
    }*/
    
    i=8;
    while(i-- && !newjtag)
    {
        JTAGPulse();
    }

    i=0;

    while(length--)
    {
        i++;
        if(!length || (i==8 && newjtag))
            SET_JTAG_TMS();

        SET_JTAG_TCK();
        NOP();

        // read TDO
        if(READ_JTAG_TDO())
            temp |= _BV(7);
        
        CLEAR_JTAG_TCK();

        if(i==8)
        {
            // load buffer with byte
            *buffer++ = temp;
            i=0;

            if(length && newjtag)
                updateToShiftDR();
        }
        temp>>=1;
    }

    // change to the Update state
    JTAGUpdateDRState();
}
#endif


void waitForFuseEep(void)
{
    while(!(JTAGDRScanRead(0x3300, 15) & (1<<9)));
    
}

JTAGCONST ProgEEPROMPageCommand1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x11, 0x23,
                                        0x00 };

JTAGCONST ProgEEPROMPageCommand2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x77,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                        0x00 };

JTAGCONST ProgEEPROMPageCommand3[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x31,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                        JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                        0x00 };

void AVRJTAGProgramEEPROMByte(u_char byte, u_int address)
{
    JTAGNewScan(ProgEEPROMPageCommand1);
    JTAGDRScan(0x0700 | (u_char)(address>>8), 15);
    {
        JTAGDRScan(0x0300 | (u_char)address, 15);
        JTAGDRScan(0x1300 | byte, 15);
        JTAGNewScan(ProgEEPROMPageCommand2);
    }
    JTAGNewScan(ProgEEPROMPageCommand3);

    waitForFuseEep();
}


JTAGCONST ReadFlashPageCommand1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x02, 0x23,
                                        0x00 };

JTAGCONST ReadFlashPageCommand2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x02, 0x32,
                                        0x00 };


// address is a word address, not byte address
// length is in words, not bytes
void AVRJTAGReadFlash(u_char * buffer, u_long address, u_int length)
{
    u_int i;
    
    JTAGNewScan(ReadFlashPageCommand1);
    for(i=0; i<length; i++)
    {
		AVRJTAGScan3ByteAddress(address);
    
        JTAGNewScan(ReadFlashPageCommand2);
        *buffer++ = JTAGDRScanRead(0x3600, 15);
        *buffer++ = JTAGDRScanRead(0x3700, 15);
        address++;
    }
}

JTAGCONST ReadEEPROMPageCommand1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x03, 0x23,
                                        0x00 };

JTAGCONST ReadEEPROMPageCommand2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x32,
                                        0x00 };

// buffer must be at least as large as length
void AVRJTAGReadEEPROM(u_char * buffer, u_int address, u_int length)
{
    u_int i;
    
    JTAGNewScan(ReadEEPROMPageCommand1);
    for(i=0; i<length; i++)
    {
        JTAGDRScan(0x0700 | (u_char)(address>>8), 15);
        JTAGDRScan(0x0300 | (u_char)address, 15);
        JTAGDRScan(0x3300 | (u_char)address, 15);
        JTAGNewScan(ReadEEPROMPageCommand2);
        *buffer++ = (u_char)JTAGDRScanRead(0x3300, 15);
        address++;
    }
}


JTAGCONST ReadSignature1[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x08, 0x23,
                                0x00 };

JTAGCONST ReadSignature2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x32,
                                JTAG_DROUT_SCAN | (15-1), 0x00, 0x33,
                                0x00 };

u_char AVRJTAGReadSignature(u_char address)
{

    JTAGNewScan(ReadSignature1);
    JTAGDRScan(0x0300 | (u_char)address, 15);
    return JTAGNewScan(ReadSignature2);
}


JTAGCONST WriteLockBits1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x20, 0x23,
                                0x00 };

JTAGCONST WriteLockBits2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x31,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                0x00 };

void AVRJTAGWriteLockBits(u_char lockBits)
{
    JTAGNewScan(WriteLockBits1);
    JTAGDRScan(0x13C0 | lockBits, 15);
    JTAGNewScan(WriteLockBits2);

    waitForFuseEep();
}

JTAGCONST WriteAnyFuseBits[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x40, 0x23,
                                0x00 };
/*JTAGCONST WriteHighFuseBits1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x40, 0x23,
                                0x00 };*/

JTAGCONST WriteHighFuseBits2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x35,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x37,
                                0x00 };




/*JTAGCONST WriteExtendedFuseBits1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x40, 0x23,
                                0x00 };*/

JTAGCONST WriteExtendedFuseBits2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x3B,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x39,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x3B,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x3B,
                                0x00 };



/*JTAGCONST WriteFuseBits1[] = {  JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                JTAG_DRRTI_SCAN | (15-1), 0x40, 0x23,
                                0x00 };*/

JTAGCONST WriteFuseBits2[] = {  JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x31,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                JTAG_DRRTI_SCAN | (15-1), 0x00, 0x33,
                                0x00 };





void AVRJTAGWriteCombFuseBits(u_char fusebyte, void * parameter)
{
    JTAGNewScan(WriteAnyFuseBits);
    JTAGDRScan(0x1300 | fusebyte, 15);
    JTAGNewScan(parameter);

    waitForFuseEep();
}


JTAGCONST ReadCalibByteCommand1[] = {   JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DRRTI_SCAN | (15-1), 0x08, 0x23,
                                        0x00 };

JTAGCONST ReadCalibByteCommand2[] = {   JTAG_DRRTI_SCAN | (15-1), 0x00, 0x36,
                                        JTAG_DROUT_SCAN | (15-1), 0x00, 0x37,
                                        0x00 };
    
u_char AVRJTAGReadCalibrationByte(u_char address)
{

    JTAGNewScan(ReadCalibByteCommand1);
    JTAGDRScan(0x0300 | (u_char)(address), 15);

    return JTAGNewScan(ReadCalibByteCommand2);
}


JTAGCONST ReadDeviceIDCommand[] = {JTAG_IR_SCAN | (4-1), 0x01, 0x00,
                                    0x00 };

u_long JTAGReadDeviceID(void)
{
    u_char length = 32;
    u_long TDISeq = 0;
    u_long TDOSeq = 0l;
    u_long tdoval;
    
    JTAGNewScan(ReadDeviceIDCommand);
    
    // change to the Shift-DR state
    JTAGShiftDRState();
            
    tdoval = 1;
    
    while(length--)
    {
        if(!length)
            SET_JTAG_TMS();
            
        CLEAR_JTAG_TDI();

        if(TDISeq & 0x01)
            SET_JTAG_TDI();

        SET_JTAG_TCK();
        NOP();

        // read TDO
        if(READ_JTAG_TDO())
            TDOSeq |= tdoval;
        
        CLEAR_JTAG_TCK();

        tdoval <<= 1;
        TDISeq >>=1;
    }


    // change to the Update-DR state
    JTAGUpdateDRState();

    return TDOSeq;
}


JTAGCONST EnterProgrammingModeCommand[] = {JTAG_IR_SCAN | (4-1), 0x0C, 0x00,
                                    JTAG_DR_SCAN | (1-1), 1, 0x00,
                                    JTAG_IR_SCAN | (4-1), 0x04, 0x00,
                                    JTAG_DR_SCAN | (16-1), 0x70, 0xA3,
                                    0x00 };

#define JTAG_ATMEL_MANUFACTURER_ID 0x3F
#define JTAG_MANUFACTURER_ID_MASK 0xFFF


int AVRJTAGEnterProgrammingMode(void)
{
    u_long deviceid;
    u_char i;
    

    // reset the target - enabling JTAG if the JTD bit is set in firmware
    DRIVE_RESET();

    // make the JTAG pins output
    SET_JTAG_OUTPUT();

    delay_ms(1);

    // put the bus in the idle state
    SET_JTAG_TMS();
    nop();
    
    for(i=0; i<5; i++)
    {
        JTAGPulse();
    }

    CLEAR_JTAG_TMS();

    JTAGPulse();


    // verify the device ID command matches Atmel
    // if not, make the bus pins input and return
    deviceid = JTAGReadDeviceID();

    if((deviceid & JTAG_MANUFACTURER_ID_MASK) != JTAG_ATMEL_MANUFACTURER_ID)
    {
        // make the JTAG pins input
        CLEAR_JTAG_OUTPUT();
        
        RELEASE_RESET();
        return -1;
    }
  
    // enter programming mode
    JTAGNewScan(EnterProgrammingModeCommand);


    u_char sig1, sig2;
    sig1 = AVRJTAGReadSignature(1);
    sig2 = AVRJTAGReadSignature(2);
    
    if((sig1 <= 0x97 && sig2 <= 0x02) || (sig1 == 0x94 && sig2 <= 0x04))
        newjtag=0;
    else
        newjtag=1;

    return 0;
}

JTAGCONST LeaveProgrammingModeCommand[] = {JTAG_IR_SCAN | (4-1), 0x05, 0x00,
                                        JTAG_DR_SCAN | (15-1), 0x00, 0x23,
                                        JTAG_DR_SCAN | (15-1), 0x00, 0x33,
                                        JTAG_IR_SCAN | (4-1), 0x04, 0x00,
                                        JTAG_DR_SCAN | (16-1), 0x00, 0x00,
                                        JTAG_IR_SCAN | (4-1), 0x0C, 0x00,
                                        JTAG_DR_SCAN | (1-1), 0x00, 0x00,
                                        0x00 };

void AVRJTAGLeaveProgrammingMode(void)
{
    JTAGNewScan(LeaveProgrammingModeCommand);

    // make the JTAG pins input with pullup
    CLEAR_JTAG_OUTPUT();
    SET_JTAG_PULLUPS();
    RELEASE_RESET();
}



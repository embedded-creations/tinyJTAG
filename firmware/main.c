/*
 * USBasp - USB in-circuit programmer for Atmel AVR controllers
 *
 * Thomas Fischl <tfischl@gmx.de>
 *
 * License........: GNU GPL v2 (see Readme.txt)
 * Target.........: ATMega8 at 12 MHz
 * Creation Date..: 2005-02-20
 * Last change....: 2009-02-28
 *
 * PC2 SCK speed option.
 * GND  -> slow (8khz SCK),
 * open -> software set speed (default is 375kHz SCK)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "usbasp.h"
#include "usbdrv.h"
#include "isp.h"
#include "clock.h"
#include <util/delay.h>
#include "tpi.h"
#include "tpi_defs.h"

#include "jtag.h"
#include "hardware.h"

//#define UART_DEBUG

#ifdef UART_DEBUG
#include "buffuart.h"
#endif


static uchar replyBuffer[8];

static uchar prog_state = PROG_STATE_IDLE;
static uchar prog_sck = USBASP_ISP_SCK_AUTO;

static uchar prog_address_newmode = 0;
static unsigned long prog_address;
static unsigned int prog_nbytes = 0;
static unsigned int prog_pagesize;
static uchar prog_blockflags;
static uchar prog_pagecounter;

unsigned int flashpagesize;

uchar usbFunctionSetup(uchar data[8]) {

	uchar len = 0;

#ifdef UART_DEBUG
	TransmitHex(data[1]);
#endif
	
	if (data[1] == USBASP_FUNC_CONNECT) {

#if 0
	    /* set SCK speed */
		if ((PINC & (1 << PC2)) == 0) {
			ispSetSCKOption(USBASP_ISP_SCK_8);
		} else {
			ispSetSCKOption(prog_sck);
		}
#endif

		/* set compatibility mode of address delivering */
		prog_address_newmode = 0;

		ledRedOn();
		ispConnect();

	} else if (data[1] == USBASP_FUNC_DISCONNECT) {
		ispDisconnect();
		ledRedOff();

	} else if (data[1] == USBASP_FUNC_TRANSMIT) {
		// debug - print out contents of message
#ifdef UART_DEBUG
		TransmitHex(data[2]);
		TransmitHex(data[3]);
		TransmitHex(data[4]);
		TransmitHex(data[5]);
#endif
		replyBuffer[0] = 0;
		replyBuffer[1] = 0;
		replyBuffer[2] = 0;
		replyBuffer[3] = interpretSPICommand(&data[2]);
		len = 4;
		
	} else if (data[1] == USBASP_FUNC_READFLASH) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_READFLASH;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_READEEPROM) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_READEEPROM;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_ENABLEPROG) {
		replyBuffer[0] = ispEnterProgrammingMode();
		len = 1;

	} else if (data[1] == USBASP_FUNC_WRITEFLASH) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_pagesize = data[4];
		prog_blockflags = data[5] & 0x0F;
		prog_pagesize += (((unsigned int) data[5] & 0xF0) << 4);
		if (prog_blockflags & PROG_BLOCKFLAG_FIRST) {
			prog_pagecounter = prog_pagesize;
		}
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_WRITEFLASH;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_WRITEEEPROM) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_pagesize = 0;
		prog_blockflags = 0;
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_WRITEEEPROM;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_SETLONGADDRESS) {

		/* set new mode of address delivering (ignore address delivered in commands) */
		prog_address_newmode = 1;
		/* set new address */
		prog_address = *((unsigned long*) &data[2]);

	} else if (data[1] == USBASP_FUNC_SETISPSCK) {

		/* set sck option */
		prog_sck = data[2];
		replyBuffer[0] = 0;
		len = 1;

#if 0
		} else if (data[1] == USBASP_FUNC_TPI_CONNECT) {
		tpi_dly_cnt = data[2] | (data[3] << 8);

		/* RST high */
		ISP_OUT |= (1 << ISP_RST);
		ISP_DDR |= (1 << ISP_RST);

		clockWait(3);

		/* RST low */
		ISP_OUT &= ~(1 << ISP_RST);
		ledRedOn();

		clockWait(16);
		tpi_init();
	
	} else if (data[1] == USBASP_FUNC_TPI_DISCONNECT) {

		tpi_send_byte(TPI_OP_SSTCS(TPISR));
		tpi_send_byte(0);

		clockWait(10);

		/* pulse RST */
		ISP_OUT |= (1 << ISP_RST);
		clockWait(5);
		ISP_OUT &= ~(1 << ISP_RST);
		clockWait(5);

		/* set all ISP pins inputs */
		ISP_DDR &= ~((1 << ISP_RST) | (1 << ISP_SCK) | (1 << ISP_MOSI));
		/* switch pullups off */
		ISP_OUT &= ~((1 << ISP_RST) | (1 << ISP_SCK) | (1 << ISP_MOSI));

		ledRedOff();
	
	} else if (data[1] == USBASP_FUNC_TPI_RAWREAD) {
		replyBuffer[0] = tpi_recv_byte();
		len = 1;
	
	} else if (data[1] == USBASP_FUNC_TPI_RAWWRITE) {
		tpi_send_byte(data[2]);
	
	} else if (data[1] == USBASP_FUNC_TPI_READBLOCK) {
		prog_address = (data[3] << 8) | data[2];
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_TPI_READ;
		len = 0xff; /* multiple in */
	
	} else if (data[1] == USBASP_FUNC_TPI_WRITEBLOCK) {
		prog_address = (data[3] << 8) | data[2];
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_TPI_WRITE;
		len = 0xff; /* multiple out */
#endif
	} else if (data[1] == USBASP_FUNC_GETCAPABILITIES) {
		replyBuffer[0] = USBASP_CAP_0_TPI;
		replyBuffer[1] = 0;
		replyBuffer[2] = 0;
		replyBuffer[3] = 0;
		len = 4;
	}

	usbMsgPtr = replyBuffer;

#ifdef UART_DEBUG
	TransmitString("\r\n");
#endif

	return len;
}

uchar usbFunctionRead(uchar *data, uchar len) {

	/* check if programmer is in correct read state */
	if ((prog_state != PROG_STATE_READFLASH) && (prog_state
			!= PROG_STATE_READEEPROM) && (prog_state != PROG_STATE_TPI_READ)) {
		return 0xff;
	}

    if (prog_state == PROG_STATE_READFLASH) {
        AVRJTAGReadFlash(data, prog_address/2, len/2);
    } else {
        AVRJTAGReadEEPROM(data, prog_address, len);
    }

    prog_address += len;

	/* last packet? */
	if (len < 8) {
		prog_state = PROG_STATE_IDLE;
	}

	return len;
}

uchar usbFunctionWrite(uchar *data, uchar len) {

	uchar retVal = 0;
	uchar i;

	/* check if programmer is in correct write state */
	if ((prog_state != PROG_STATE_WRITEFLASH) && (prog_state
			!= PROG_STATE_WRITEEEPROM) && (prog_state != PROG_STATE_TPI_WRITE)) {
		return 0xff;
	}

#if 0
	if (prog_state == PROG_STATE_TPI_WRITE)
	{
		tpi_write_block(prog_address, data, len);
		prog_address += len;
		prog_nbytes -= len;
		if(prog_nbytes <= 0)
		{
			prog_state = PROG_STATE_IDLE;
			return 1;
		}
		return 0;
	}
#endif

#if 0
#ifdef UART_DEBUG
	if (prog_state == PROG_STATE_WRITEFLASH)
        TransmitString("WRFL\r\n");

    TransmitString("len:");
    TransmitHex(len);
    TransmitString("\r\n");

    TransmitString("ctr:");
    TransmitHex(prog_pagecounter/256);
    TransmitHex(prog_pagecounter);
    TransmitString("\r\n");

    TransmitString("size:");
    TransmitHex(prog_pagesize/256);
    TransmitHex(prog_pagesize);
    TransmitString("\r\n");

    TransmitString("adx:");
    TransmitHex(prog_address/256/256);
    TransmitHex(prog_address/256);
    TransmitHex(prog_address);
    TransmitString("\r\n");
#endif

#endif

    flashpagesize = prog_pagesize;

    for (i = 0; i < len; i++) {

		if (prog_state == PROG_STATE_WRITEFLASH) {
			/* Flash */

			if (prog_pagesize == 0) {
				/* not paged */
#ifdef UART_DEBUG
			    TransmitString("NP write not supported");
#endif
			    return 0xff;
			} else {
				/* paged */
                // load one word into the flash buffer
			    AVRJTAGLoadFlashPage(data + i, prog_address/2, 1);
			    i++;

				prog_pagecounter--;
				prog_pagecounter--;

				if (prog_pagecounter == 0) {
				    AVRJTAGWriteFlashPage(prog_address/2);
					prog_pagecounter = prog_pagesize;
				}
			}

		} else {
			/* EEPROM */
		    AVRJTAGProgramEEPROMByte(data[i], prog_address);
		}

        if (prog_state == PROG_STATE_WRITEFLASH) {
            prog_nbytes-=2;
        } else {
            prog_nbytes--;
        }

		if (prog_nbytes == 0) {
			if ((prog_blockflags & PROG_BLOCKFLAG_LAST) && (prog_pagecounter
					!= prog_pagesize)) {

				/* last block and page flush pending, so flush it now */
		        if (prog_state == PROG_STATE_WRITEFLASH) {
		            AVRJTAGWriteFlashPage(prog_address/2);
		        }

		        // No need to flush EEPROM
            }

            prog_state = PROG_STATE_IDLE;
			retVal = 1; // Need to return 1 when no more data is to be received
		}

        if (prog_state == PROG_STATE_WRITEFLASH) {
            prog_address += 2;
        } else {
            prog_address++;
        }
	}

	return retVal;
}

int main(void) {
	uchar i, j;

	/* no pullups on USB and ISP pins */
	//PORTD = 0;
	//PORTB = 0;
	/* all outputs except PD2 = INT0 */
	/// LB - different pins on STK500 board
	//DDRD = ~(1 << 2);
	// make JTAG pins inputs with pullups	
	SET_JTAG_PULLUPS();

	/* output SE0 for USB reset */
	/// LB - different pins on STK500 board
	//DDRB = ~0;
	j = 0;
	/* USB Reset by device only required on Watchdog Reset */
	while (--j) {
		i = 0;
		/* delay >10ms for USB reset */
		while (--i)
			;
	}
	/* all USB and ISP pins inputs */
	//DDRB = 0;

	/// LB - LED pins are different from usbasp to sp duo - conflict: SP duo uses these for JTAG
	/* all inputs except PC0, PC1 */
	//DDRC = 0x03;
	//PORTC = 0xfe;
	SET_LED_OUTPUT();
	LED_OFF();

	/* init timer */
	clockInit();

	
#ifdef UART_DEBUG
    // init debug uart
    setupUART();

    TransmitString("\r\n\n***\r\nstarting up\r\n");
#endif
	
	// USB Re-Enumeration
	usbDeviceDisconnect();
	while(--i){         // fake USB disconnect for > 250 ms
	    wdt_reset();    // if watchdog is active, reset it
	    _delay_ms(1);   // library call -- has limited range
	}
	usbDeviceConnect();

	/* main event loop */
	usbInit();
	sei();
	for (;;) {
		usbPoll();
	}
	return 0;
}


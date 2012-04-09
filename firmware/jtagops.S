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
#include "hardware.h"

.text
.global JTAGDRScan



;u_long JTAGDRScan(u_int TDISeq, u_char length)
JTAGDRScan:
; shift to Shift-DR, set TMS to 0
    ; clear TDI
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TDI
    ; set TMS
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TMS
    ; cycle TCK
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    ; clear TMS
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TMS
    ; cycle TCK
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    ; cycle TCK
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK

; shift in the data, leaving Shift-DR on the last bit
loop:
    ; dec length
    dec r22
    ; brne + 2
    brne dontsetTMS
    ; set TMS
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TMS

dontsetTMS:
    ; clear TDI
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TDI
    ; sbrc data low
    sbrc r24, 0
    ; set TDI
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TDI

    ; cycle TCK
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK


    ; lsr high data byte
    lsr r25
    ; ror low data byte
    ror r24

    ; test length
    tst r22
    ; brne loop
    brne loop

; change to Update-DR state, and do a Run-Test
    ; clear TDI
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TDI
    ; set TMS
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TMS
    ; cycle TCK
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    ; clear TMS
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TMS
    ; cycle TCK
    sbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK
    nop
    cbi _SFR_IO_ADDR(JTAG_PORT), JTAG_TCK

ret






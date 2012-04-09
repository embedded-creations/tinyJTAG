#ifndef _AVRDEVICECODES_H_
#define _AVRDEVICECODES_H_

/* 0x00-0x0F reserved */

/* 0x10-0x1F ATtiny 1K devices */

#define ATTINY10    0x10
#define ATTINY11    0x11
#define ATTINY12    0x12
#define ATTINY15    0x13

/* 0x20-0x2F ATtiny 2K devices */

#define ATTINY22    0x20
#define ATTINY26    0x21
#define ATTINY28    0x22

/* 0x30-0x3F ATclassic 1K devices */

#define AT90S1200   0x33

/* 0x40-0x4F ATclassic 2K devices */

#define AT90S2313   0x40
#define AT90S2323   0x41
#define AT90S2333   0x42
#define AT90S2343   0x43

/* 0x50-0x5F ATclassic 4K devices */

#define AT90S4414   0x50
#define AT90S4433   0x51
#define AT90S4434   0x52

/* 0x60-0x6F ATclassic 8K devices */

#define AT90S8515   0x60
#define AT90S8535   0x61
#define AT90C8534   0x62
#define ATMEGA8515  0x63
#define ATMEGA8535  0x64

/* 0x70-0x7F ATmega 8K devices */

#define ATMEGA8     0x70

/* 0x80-0x8F ATmega 16K devices */

#define ATMEGA161   0x80
#define ATMEGA163   0x81
#define ATMEGA16    0x82
#define ATMEGA162   0x83
#define ATMEGA169   0x84

/* 0x90-0x9F ATmega 32K devices */

#define ATMEGA323   0x90
#define ATMEGA32    0x91

/* 0xA0-0xAF ATmega 64K devices */

/* 0xB0-0xBF ATmega 128K devices */

#define ATMEGA103   0xB1
#define ATMEGA128   0xB2

/* 0xC0-0xCF not used */

/* 0xD0-0xDF Other devices */
#define AT86RF401   0xD0

/* 0xE0-0xEF AT89 devices */
#define AT89START   0xE0
#define AT89S51	    0xE0
#define AT89S52	    0xE1

/* 0xF0-0xFF reserved */

#define DEFAULTDEVICE	AT90S8515


#endif


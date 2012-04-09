/******************************************************************************

	Title:		Interrupt-Driven Buffered UART for AT90S8535
	Author: 	Atmel Appnote AVR-306 (modified for AVR-GCC
				by Amedee Beaudoin www.embedded-creations.com)
	Date: 		5/2002
	Purpose: 	
	Software:	AVR-GCC to compile
	Hardware:	AT90S8535

******************************************************************************/

#ifndef _BUFFUART_H_
#define _BUFFUART_H_

#include "clock.h"


#define UART_BAUD_RATE      57600

#define UART_BAUD_SELECT 		(F_CPU/(UART_BAUD_RATE*16l)-1)

#define UART_TX_BUFFER_SIZE		32
#define UART_TX_BUFFER_MASK     UART_TX_BUFFER_SIZE-1

#define UART_RX_BUFFER_SIZE		8
#define UART_RX_BUFFER_MASK     UART_RX_BUFFER_SIZE-1


void setupUART( void ) ;
char ReceiveByte( void ) ;
void TransmitByte( char data ) ;
unsigned char DataInReceiveBuffer( void ) ;
void TransmitString( char *stringPointer ) ;
void TransmitHex(unsigned char data);





#endif

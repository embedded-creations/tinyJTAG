#include <avr/io.h>
#include <avr/interrupt.h>

#include "buffUART.h"

static unsigned char 			UART_RxBuf[UART_RX_BUFFER_SIZE] ;
static volatile unsigned int 	UART_RxHead ;
static volatile unsigned int 	UART_RxTail ;
static unsigned char 			UART_TxBuf[UART_TX_BUFFER_SIZE] ;
static volatile unsigned char 	UART_TxHead ;
static volatile unsigned char 	UART_TxTail ;


/* sends a null-terminated string of length <256 to the UART */
void TransmitString( char *stringPointer )
{
	unsigned char index = 0 ;
	while( stringPointer[index] != '\0' )
	{
		TransmitByte( stringPointer[index++] ) ;
	}
	
	return ;
}

void setupUART( void )
{
    /* enable RxD/TxD and interrupt on character received */
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);
    
    /* set baud rate */
	UBRRH = (unsigned char)UART_BAUD_SELECT >> 8;
	UBRRL = (unsigned char)UART_BAUD_SELECT;

	UART_RxTail = 0;						/* flush the transmit and receive buffers */

	UART_RxHead = 0;
	UART_TxTail = 0;
	UART_TxHead = 0;

	return ;
}



SIGNAL(SIG_UART_RECV)
{
	unsigned char data;
	unsigned int tmphead;

	data = UDR ; 								/* read the received data */

	
	tmphead = (UART_RxHead + 1) ;						/* calculate buffer index */
	//tmphead &= UART_RX_BUFFER_MASK  ;
    if(tmphead == UART_RX_BUFFER_SIZE)
        tmphead = 0;
	
	UART_RxHead = tmphead; 								/* store new index */
	
	if ( tmphead == UART_RxTail )
	{
		/* ERROR! Receive buffer overflow */
	}
	
	UART_RxBuf[tmphead] = data; /* store received data in buffer */
}

SIGNAL( SIG_UART_DATA )
{
	unsigned char tmptail;

	/* check if all data is transmitted */
	if ( UART_TxHead != UART_TxTail )
	{
		tmptail = ( UART_TxTail + 1 ) ;					/* calculate buffer index */
		tmptail &= UART_TX_BUFFER_MASK ;
		UART_TxTail = tmptail; 							/* store new index */
		UDR = UART_TxBuf[tmptail];				/* start transmition */
	}
	else
	{
														/* Buffer Empty - 		*/
		UCSRB &= ~_BV(UDRIE); 							/* disable UDRE interrupt */
	}
}



char ReceiveByte( void )
{
	unsigned int tmptail;
	
	while ( UART_RxHead == UART_RxTail ) ;				/* wait for incomming data */
	
	tmptail = ( UART_RxTail + 1 ) ;
	//tmptail &= UART_RX_BUFFER_MASK ;					/* calculate buffer index */
    if(tmptail == UART_RX_BUFFER_SIZE)
        tmptail = 0;
	UART_RxTail = tmptail; 								/* store new index */
	return UART_RxBuf[tmptail]; 						/* return data */
}


void TransmitByte( char data )
{
	unsigned char tmphead;
	
	tmphead = ( UART_TxHead + 1 ) ;
	tmphead &= UART_TX_BUFFER_MASK ;					/* calculate buffer index */
	
#if 1
	while ( tmphead == UART_TxTail );					/* wait for free space in buffer */
#else
	// don't wait for space if buffer is full
	if(tmphead == UART_TxTail)
	    return;
#endif
	
	UART_TxBuf[tmphead] = data; 						/* store data in buffer */
	UART_TxHead = tmphead; 								/* store new index */
	UCSRB |= _BV(UDRIE) ;								/* enable UDRE interrupt */

	return ;
}


unsigned char DataInReceiveBuffer( void )
{
	return ( UART_RxHead != UART_RxTail ); 			/* return 0 (FALSE) if the receive buffer is empty */
}

#if 1
void TransmitHex(unsigned char data)
{
    unsigned char temp = data & 0xF0;
    temp >>= 4;

    if(temp >= 0x0A)
        TransmitByte('A' + temp - 0x0A);
    else TransmitByte('0' + temp);

    temp = data & 0x0F;
    if(temp >= 0x0A)
        TransmitByte('A' + temp - 0x0A);
    else TransmitByte('0' + temp);
}
#endif


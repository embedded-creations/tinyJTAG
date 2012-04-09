#define SPBETA1 1
#define SPV10   2
#define TINY85	3
#define HARDWAREDEF TINY85

#if HARDWAREDEF == SPBETA1

#define RESETPIN 0
#define RESETDDR DDRB
#define SPIDDR  DDRB
#define SPIPORT PORTB
#define SCKPIN  5
#define MOSIPIN 3
#define MISOPIN 4
#define SSPIN   2

#define JTAG_PORT   PORTC
#define JTAG_DDR    DDRC
#define JTAG_PINS   PINC
#define JTAG_TCK    0
#define JTAG_TMS    2
#define JTAG_TDI    3
#define JTAG_TDO    1

#define SET_JTAG_TCK() asm volatile("sbi 0x15, 0")
#define SET_JTAG_TMS() asm volatile("sbi 0x15, 2")
#define SET_JTAG_TDI() asm volatile("sbi 0x15, 3")
#define CLEAR_JTAG_TCK() asm volatile("cbi 0x15, 0")
#define CLEAR_JTAG_TMS() asm volatile("cbi 0x15, 2")
#define CLEAR_JTAG_TDI() asm volatile("cbi 0x15, 3")
#endif


#if HARDWAREDEF == SPV10
#define RESETPIN 5
#define RESETDDR DDRC
#define SPIDDR  DDRB
#define SPIPORT PORTB
#define SCKPIN  5
#define MOSIPIN 3
#define MISOPIN 4
#define SSPIN   2

#define LEDTARGPIN 4
#define LEDTARGDDR DDRC

#define JTAG_PORT   PORTC
#define JTAG_DDR    DDRC
#define JTAG_PINS   PINC
#define JTAG_TCK    1
#define JTAG_TMS    2
#define JTAG_TDI    0
#define JTAG_TDO    3

#define SET_JTAG_TCK()      (JTAG_PORT |= _BV(JTAG_TCK))
#define SET_JTAG_TMS()      (JTAG_PORT |= _BV(JTAG_TMS))
#define SET_JTAG_TDI()      (JTAG_PORT |= _BV(JTAG_TDI))
#define CLEAR_JTAG_TCK()    (JTAG_PORT &= ~_BV(JTAG_TCK))
#define CLEAR_JTAG_TMS()    (JTAG_PORT &= ~_BV(JTAG_TMS))
#define CLEAR_JTAG_TDI()    (JTAG_PORT &= ~_BV(JTAG_TDI))

#define CTSPORT PORTD
#define CTSDDR  DDRD
#define CTSPIN  4

#define RELEASE_LEDTARG() LEDTARGDDR &= ~_BV(LEDTARGPIN)
#define DRIVE_LEDTARG() LEDTARGDDR |= _BV(LEDTARGPIN)

#define SET_LED_OUTPUT()    
#define LED_OFF()   

/* macros for gpio functions */
#define ledRedOn()    
#define ledRedOff()   
#define ledGreenOn()  
#define ledGreenOff() 

#define READ_JTAG_TDO()         bit_is_set(JTAG_PINS, JTAG_TDO)

#define SET_JTAG_OUTPUT()       (JTAG_DDR |= _BV(JTAG_TCK) | _BV(JTAG_TMS) | _BV(JTAG_TDI))

#define CLEAR_JTAG_OUTPUT()     (JTAG_DDR &= ~(_BV(JTAG_TCK) | _BV(JTAG_TMS) | _BV(JTAG_TDI)))

#endif


#if HARDWAREDEF == TINY85


#define LEDTARGPIN 4
#define LEDTARGDDR DDRC

#define JTAG_PORT   PORTB
#define JTAG_DDR    DDRB
#define JTAG_PINS   PINB
#define JTAG_TCK    1
#define JTAG_TMS    3
#define JTAG_TDI    4
#define JTAG_TDO    5

#define SET_JTAG_TCK()      (JTAG_PORT |= _BV(JTAG_TCK))
#define SET_JTAG_TMS()      (JTAG_PORT |= _BV(JTAG_TMS))
#define SET_JTAG_TDI()      (JTAG_PORT |= _BV(JTAG_TDI))
#define CLEAR_JTAG_TCK()    (JTAG_PORT &= ~_BV(JTAG_TCK))
#define CLEAR_JTAG_TMS()    (JTAG_PORT &= ~_BV(JTAG_TMS))
#define CLEAR_JTAG_TDI()    (JTAG_PORT &= ~_BV(JTAG_TDI))

#define CTSPORT PORTD
#define CTSDDR  DDRD
#define CTSPIN  4

#define RELEASE_LEDTARG()   (LEDTARGDDR &= ~_BV(LEDTARGPIN))
#define DRIVE_LEDTARG()     (LEDTARGDDR |= _BV(LEDTARGPIN))

#define SET_LED_OUTPUT()
#define LED_OFF()

/* macros for gpio functions */
#define ledRedOn()
#define ledRedOff()
#define ledGreenOn()
#define ledGreenOff()

#define READ_JTAG_TDO()         bit_is_set(JTAG_PINS, JTAG_TDO)

#define SET_JTAG_OUTPUT()       (JTAG_DDR |= _BV(JTAG_TCK) | _BV(JTAG_TMS) | _BV(JTAG_TDI))

#define CLEAR_JTAG_OUTPUT()     (JTAG_DDR &= ~(_BV(JTAG_TCK) | _BV(JTAG_TMS) | _BV(JTAG_TDI)))
#define SET_JTAG_PULLUPS()      (JTAG_PORT |= _BV(JTAG_TCK) | _BV(JTAG_TMS) | _BV(JTAG_TDI) )


#endif


#ifndef RELEASE_RESET
#define RELEASE_RESET() //RESETDDR &= ~_BV(RESETPIN);
#endif
#ifndef DRIVE_RESET
#define DRIVE_RESET() //RESETDDR |= _BV(RESETPIN)
#endif



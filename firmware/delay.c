#include "delay.h"

#if 0
void delay_char(unsigned char number_of_loops) 
{
/* 3 cpu cycles per loop + 10 cycles overhead when a constant is passed. */
  __asm__ volatile ( "cp  %A0,__zero_reg__ \n\t"  \
                     "breq L_EXIT_%=       \n\t"  \
                     "L_%=:                \n\t"  \
                     "dec %A0              \n\t"  \
                     "brne L_%=            \n\t"  \
                     "L_EXIT_%=:           \n\t"  \
                     : /* NO OUTPUT */            \
                     : "r" (number_of_loops)      \
                   );                            
                                            
return;
}
#endif
 
void delay_short(unsigned short number_of_loops) 
{
/* 4 cpu cycles per loop + 12 cycles overhead when a constant is passed. */
  __asm__ volatile ( "cp  %A0,__zero_reg__ \n\t"  \
                     "cpc %B0,__zero_reg__ \n\t"  \
                     "breq L_EXIT_%=       \n\t"  \
                     "L_%=:                \n\t"  \
                     "sbiw r24,1           \n\t"  \
                     "brne L_%=            \n\t"  \
                     "L_EXIT_%=:           \n\t"  \
                     : /* NO OUTPUT */            \
                     : "w" (number_of_loops)      \
                   );                            

                                            
return;
}

void delay_long(unsigned long number_of_loops) 
{
/* 6 cpu cycles per loop + 20 cycles overhead when a constant is passed. */
  __asm__ volatile ( "cp  %A0,__zero_reg__ \n\t"  \
                     "cpc %B0,__zero_reg__ \n\t"  \
                     "cpc %C0,__zero_reg__ \n\t"  \
                     "cpc %D0,__zero_reg__ \n\t"  \
                     "breq L_EXIT_%=       \n\t"  \
                     "L_%=:                \n\t"  \
                     "subi %A0,lo8(-(-1))  \n\t"  \
                     "sbci %B0,hi8(-(-1))  \n\t"  \
                     "sbci %C0,hlo8(-(-1)) \n\t"  \
                     "sbci %D0,hhi8(-(-1)) \n\t"  \
                     "brne L_%=            \n\t"  \
                     "L_EXIT_%=:           \n\t"  \
                     : /* NO OUTPUT */            \
                     : "w" (number_of_loops)      \
                   );                             \
                                              
return;
}


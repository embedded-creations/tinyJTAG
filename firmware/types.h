#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#define nop() asm volatile("nop"::)


#define BYTE1(x) *(((u_char*)&x)+0)
#define BYTE2(x) *(((u_char*)&x)+1)
#define BYTE3(x) *(((u_char*)&x)+2)
#define BYTE4(x) *(((u_char*)&x)+3)


/*! \brief Unsigned 8-bit value. */
typedef unsigned char      u_char;
/*! \brief Unsigned 16-bit value. */
typedef unsigned short     u_short;
/*! \brief Unsigned 16-bit value. */
typedef unsigned int       u_int;
/*! \brief Unsigned 32-bit value */
typedef unsigned long      u_long;
/*! \brief Unsigned 64-bit value */
typedef unsigned long long u_longlong;
/*! \brief Void pointer */
typedef void * HANDLE;

#endif

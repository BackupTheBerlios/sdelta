/* $Id: adler32.h,v 1.8 2005/01/11 17:16:44 svinn Exp $ */

#include <sys/types.h>

#ifdef __linux__
#include <endian.h>
#elif defined(sun) || defined(__sun)
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
#ifndef __BYTE_ORDER
#define __LITTLE_ENDIAN	1234
#define __BIG_ENDIAN	4321
#ifdef _LITTLE_ENDIAN
#define __BYTE_ORDER	__LITTLE_ENDIAN
#else
#define __BYTE_ORDER	__BIG_ENDIAN
#endif /* _LITTLE_ENDIAN */
#endif /* __BYTE_ORDER */
#elif defined(_BYTE_ORDER) && !defined(__BYTE_ORDER)
/* FreeBSD-5 NetBSD DFBSD TrustedBSD */
#define __BYTE_ORDER	_BYTE_ORDER
#define __LITTLE_ENDIAN	_LITTLE_ENDIAN
#define __BIG_ENDIAN	_BIG_ENDIAN
#define __PDP_ENDIAN	_PDP_ENDIAN
#elif defined(BYTE_ORDER) && !defined(__BYTE_ORDER)
/* FreeBSD-[34] OpenBSD MirBSD */
#define __BYTE_ORDER	BYTE_ORDER
#define __LITTLE_ENDIAN	LITTLE_ENDIAN
#define __BIG_ENDIAN	BIG_ENDIAN
#define __PDP_ENDIAN	PDP_ENDIAN
#endif

#ifndef __BYTE_ORDER
#error __BYTE_ORDER is undefined
#endif

typedef union DWORD {

  u_int32_t  dword;

  struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN
      u_int16_t  low;
      u_int16_t  high;
    #else
      u_int16_t  high;
      u_int16_t  low;
    #endif
  }  word;

  struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN
      unsigned char  b0;
      unsigned char  b1;
      unsigned char  b2;
      unsigned char  b3;
    #elif __BYTE_ORDER == __BIG_ENDIAN
      unsigned char  b3;
      unsigned char  b2;
      unsigned char  b1;
      unsigned char  b0;
    #else /* __BYTE_ORDER == __PDP_ENDIAN */
      unsigned char  b2;
      unsigned char  b3;
      unsigned char  b1;
      unsigned char  b0;
    #endif
  }  byte;

} DWORD;


u_int32_t  adler32	(unsigned char *b, u_int32_t s);

#define crc_tag(crc)( ( (crc).word.low + (crc).word.high ) & 0xffff )

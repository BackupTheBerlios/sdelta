/* $Id: adler32.h,v 1.4 2004/12/18 16:53:33 svinn Exp $ */

#include <sys/types.h>

#if !defined(__FreeBSD__) && !defined(sun) && !defined(__sun)
#include <endian.h>
#endif /* !__FreeBSD__ && !sun && !__sun */

#if defined(sun) || defined(__sun)
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
#if !defined(BYTE_ORDER) && !defined(__BYTE_ORDER)
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#ifdef _LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif /* _LITTLE_ENDIAN */
#endif /* BYTE_ORDER */
#endif /* sun || __sun */

typedef union DWORD {

  u_int32_t  dword;

  struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN || BYTE_ORDER == LITTLE_ENDIAN
      u_int16_t  low;
      u_int16_t  high;
    #else 
      u_int16_t  high;
      u_int16_t  low;
    #endif
  }  word;

  struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN || BYTE_ORDER == LITTLE_ENDIAN
      unsigned char  b0;
      unsigned char  b1;
      unsigned char  b2;
      unsigned char  b3;
    #elif  __BYTE_ORDER == __PDP_ENDIAN || BYTE_ORDER == PDP_ENDIAN
      unsigned char  b2;
      unsigned char  b3;
      unsigned char  b1;
      unsigned char  b0;
    #else  /*  __BYTE_ORDER == _BIG_ENDIAN  */
      unsigned char  b3;
      unsigned char  b2;
      unsigned char  b1;
      unsigned char  b0;
    #endif  
  }  byte;

} DWORD;


u_int32_t  crc32	(unsigned char *b, u_int32_t s);

#define crc_tag(crc)( ( crc.word.low + crc.word.high ) & 0xffff )

/* $Id: */

#include <sys/types.h>
#if !defined(__FreeBSD__) && !defined(sun) && !defined(__sun)
#include <endian.h>
#endif

#if defined(sun) || defined(__sun)
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
#endif

typedef union DWORD {

  u_int32_t  dword;

  struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN || BYTE_ORDER == LITTLE_ENDIAN || defined(_LITTLE_ENDIAN)
      u_int16_t  low;
      u_int16_t  high;
    #else 
      u_int16_t  high;
      u_int16_t  low;
    #endif
  }  word;

  struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN || BYTE_ORDER == LITTLE_ENDIAN || defined(_LITTLE_ENDIAN)
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

#include <sys/types.h>
#include <stdlib.h>

/* adler32.h */
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

#define crc_tag(crc)( ( (crc).word.low + (crc).word.high ) & 0xffff )
/* end of adler32.h */

typedef struct TAG {
  u_int32_t	index;
  u_int32_t	range;
} TAG;


typedef struct INDEX {
/*  LINE		*natural;  */
  u_int32_t	*natural;
  DWORD		*crc;
  u_int32_t	 naturals;
  u_int32_t	*ordered;
  u_int32_t	 ordereds;
  TAG		*tags;
} INDEX;

#define MAX_BLOCK_SIZE 0x7f

u_int32_t  *natural_block_list  (unsigned char *, int,             int *);
DWORD      *crc_list            (unsigned char *, u_int32_t     *, int);
DWORD      *crc_list_sig        (unsigned char *, u_int32_t     *, int, int *);
void        make_index          (INDEX         *, unsigned char *, int);

/*
LINE	*natural_block_list	(unsigned char *, int,             int *);
INDEX	*make_index		(INDEX         *, unsigned char *, int);
*/

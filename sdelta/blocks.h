#include <stdlib.h>
#include <sys/types.h>
#include "adler32.h"

typedef struct LINE {
  u_int32_t	offset;
  DWORD		crc;
} LINE;



typedef struct INDEX {
  LINE		*natural;
  u_int32_t	 naturals;
  u_int32_t	*ordered;
  u_int32_t	 ordereds;
  u_int32_t	*tags;
} INDEX;


LINE	*natural_block_list	(unsigned char *, int,             int *);
INDEX	*make_index		(INDEX         *, unsigned char *, int);

#include <stdlib.h>
#include <sys/types.h>
#include "adler32.h"

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


u_int32_t  *natural_block_list  (unsigned char *, int,             int *);
DWORD      *crc_list            (unsigned char *, u_int32_t     *, int);
DWORD      *crc_list_sig        (unsigned char *, u_int32_t     *, int, int *);
void        make_index          (INDEX         *, unsigned char *, int);

/*
LINE	*natural_block_list	(unsigned char *, int,             int *);
INDEX	*make_index		(INDEX         *, unsigned char *, int);
*/

#include <sys/types.h>
#include "blocks.h"

typedef struct TO {
  size_t	 size;
  unsigned char	*buffer;	/* ptr to the file in memory	*/
  unsigned char	 sha1[20];	/* sha1 of the buffer		*/
  INDEX		 index;		/* Index without sorted data	*/
  size_t	 offset;	/* working offset into buffer	*/
  size_t	 block;		/* the curently examined block  */
  size_t	 limit;
} TO;


typedef struct FROM {
  size_t	 size;
  unsigned char *buffer;	/* ptr to the file in memory	*/
  unsigned char	 sha1[20];	/* sha1 of the buffer		*/
  INDEX		  index;
  size_t	 offset;	/* working offset into buffer	*/
  size_t	 block;		/* the currently examined block	*/
  size_t	 limit;
} FROM;


typedef struct MATCH {
  u_int32_t	line;
  u_int32_t	count;
} MATCH;


typedef struct PAIR {
  u_int32_t	from;
  u_int32_t	to;
  u_int32_t	count;
} PAIR;


typedef	struct	FOUND	{
  u_int32_t	 count;
  PAIR		*pair;
  unsigned char	*buffer;
  u_int32_t	 offset;
  u_int32_t	 size;
  unsigned char	 sha1[20];
}  FOUND;


#ifndef  FALSE
#define  FALSE 0
#endif

#ifndef  TRUE
#define  TRUE !FALSE
#endif

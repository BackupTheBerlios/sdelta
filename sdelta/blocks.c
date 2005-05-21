/*
This code, blocks.c written and copyrighted by Kyle Sallee,
creates and orders lists of dynamically sized blocks of data.
Please read LICENSE if you have not already
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "blocks.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


#ifndef SDELTA_3

/*
This is Kyle Sallee's implementation of Mark Adler's
excellent and fast 32-bit cyclic redundancy check.
An alternate example and longer explanation of adler32
is available at http://www.ietf.org/rfc/rfc1950.txt
This implementation is accurate only for block sizes
less than 257 bytes.
*/

static inline u_int32_t adler32(unsigned char *b, u_int32_t s) {
  u_int32_t	s1, s2;
  DWORD		w;

  s1 = 1;
  s2 = 0;

  for ( ; s > 0 ; s--, b++)  {
      s1   += *b;
      s2   += s1;
  }

  if  (  s2 < 0xfff10  )  while  ( s2 >= 0xfff1 )  s2 -= 0xfff1;
  else                                             s2 %= 0xfff1;

  w.word.low   =  ( u_int16_t )s1;
  w.word.high  =  ( u_int16_t )s2;

  return  w.dword;
}

#endif

#ifdef SDELTA_3

/*
#define trip_byte(a)   \
    ( (a) == 0x0a ) || \
    ( (a) == 0x00 ) || \
    ( (a) == 0x90 ) || \
    ( (a) == 0xff )

#define skip_byte(a)   \
    ( (a) == ' '  ) || \
    ( (a) == '#'  ) || \
    ( (a) == '/'  ) || \
    ( (a) == '*'  )

*/

int  trip_byte(char b)  {
  switch (b) {
    case 0x0a :
    case 0x00 :
    case 0x90 :
    case 0xff : return 1; break;
    default   : return 0; break;
  }
}

int  skip_byte(char b)  {
  switch (b) {
    case ' '  :
    case 0x09 :
    case '0'  :
    case '#'  :
    case '/'  :
    case '*'  : return 1; break;
    default   : return 0; break;
  }
}


u_int32_t       *natural_block_list(unsigned char *b, int s, u_int32_t *c) {

  u_int32_t     *list;
  int           off, blk, max;

  list =  (u_int32_t *) temp.current;

  max  =  s - 8;
  off  =  \
  blk  =  0;

  list[blk++]=off++;

  while   ( off < max ) {
    while ( off < max && ! ( trip_byte(b[off++]) ) );
    while ( off < max &&   ( trip_byte(b[off  ]) ) )  off++;
    while ( off < max &&   ( skip_byte(b[off  ]) ) )  off++;
    if    ( off < max && ! ( trip_byte(b[off  ]) ) )
      list[blk++]=off++;
  }

  list[blk  ] = s;

  max        = blk;
  off        = 0;
  blk        = 0;

  for(;max>off;off++)
    if ( list[off+1] - list[off] >= 0x0c )
         list[blk++] = list[off];

  list[blk]     = s;

  while ( 0x1000 > ( list[blk] - list[blk-1] ) )
    list[--blk] = s;

  *c            = blk++;
  temp.current += blk * sizeof(u_int32_t);
  return  list;
}


void  *order_blocks ( unsigned char *b, u_int32_t *n, int c ) {

  int           loop;
  u_int16_t     t;
  u_int16_t     *tag;
  TAG           *tags;


#if __GNUC__ >= 4
  auto int  compare_mem (const void *v0, const void *v1)  {
#else
  static int  compare_mem (const void *v0, const void *v1)  {
#endif

    return  memcmp ( b + *(u_int32_t *)v0,
                     b + *(u_int32_t *)v1, 0x1000 );
  }

  qsort(n, c, sizeof(u_int32_t), compare_mem);

  return;

}

#endif

#ifdef SDELTA_2

#define  break_byte(a) \
    ( (a) == 0x00 ) || \
    ( (a) == 0x0a ) || \
  ( ( (a) == '/'  ) && ( y < 0x04 ) ) || \
  ( ( (a) == '.'  ) && ( z < 0x04 ) )


#define  breakp()           \
  n = *p++,                 \
  ( ! ( break_byte(n) ) &&  \
  ( p < maxp ) )


#define  checkpad()                  \
  if ( n == 0 ) {                    \
    t++;                             \
    *t  =  p - b - 1;                \
    while ( ( max  >    p )    &&    \
            ( 0    ==  *p ) )  p++;  \
  }

u_int32_t	*natural_block_list(unsigned char *b, int s, u_int32_t *c) {
  u_int32_t		*r, *t;
  unsigned char		*p, *max, *maxp;
  u_int32_t		i;
  int			y, z;
  unsigned char		n;

  t    =  r    =  (u_int32_t *) temp.current;

  max  =  b + s;
  y    =  \
  z    =  0x00;

  for ( p = b ; p < max ; t++) {
    *t  =  p - b;
    for ( maxp = MIN(p + MAX_BLOCK_SIZE, max); breakp(););
    if ( n == 0x00 ) { z = y = 0; } else z++;
    if ( n == 0x0a ) {     y = 0; } else y++;
    checkpad();
  };

  *t  =  s;
  i   =  ( t - r );
  *c  =  i++;

  temp.current += i * sizeof(u_int32_t);

  return  r;
}

#endif

#ifdef SDELTA_1

u_int32_t	*natural_block_list(unsigned char *b, int s, u_int32_t *c) {
  u_int32_t		*r, *t;
  unsigned char		*p, *max, *maxp;
  u_int32_t		i;

  t    =  r    =  (u_int32_t *) temp.current;

  max  =  b + s;

  for ( p = b ; p < max ; t++) {
    *t  =  p - b;
    for ( maxp = MIN(p + MAX_BLOCK_SIZE, max); *p++ != 0x0a && p < maxp;);
  };

  *t  =  s;
  i   =  ( t - r );
  *c  =  i++;

  temp.current += i * sizeof(u_int32_t);

  return  r;
}


#endif

#ifndef SDELTA_3

DWORD	*crc_list ( unsigned char *b, u_int32_t *n, int c) {
  DWORD  *l, *list;
  int     size;

  l = list = ( DWORD * ) temp.current;

  temp.current += ( c + 1 ) * sizeof(DWORD);

  while ( c > 0 )  {
    size = n[1] - n[0];
    l->dword = adler32 ( b + *n, size );

    l++;
    n++;
    c--;
  }
  l->dword = 0xfff1fff1;

  return  list;
}

#endif

#ifdef SDELTA_2

unsigned int *list_sig ( u_int32_t *bl, unsigned int b, unsigned int *c) {

  u_int32_t	*r;
  u_int32_t	l, t;

  r = (u_int32_t *) temp.current;

  b--;

  for ( l = t = 0; l < b; l++ )
    if  ( ( bl[l+2] - bl[l] ) >= 0x0c )  r[t++] = l;

  *c  =  t;

  temp.current += t * sizeof(u_int32_t);

  return  r;
}

#endif

#ifdef SDELTA_1

unsigned int *list_sig ( u_int32_t *bl, unsigned int b, unsigned int *c) {

  u_int32_t	*r;
  u_int32_t	l, t;

  r = (u_int32_t *) temp.current;

  b--;

  for ( l = t = 0; l < b; l++ )
    if  ( ( bl[l+2] - bl[l] ) >= 0x10 )  r[t++] = l;

  *c  =  t;

  temp.current += t * sizeof(u_int32_t);

  return  r;
}

#endif

#ifndef SDELTA_3

u_int16_t   *tag_list ( DWORD *cr, unsigned int c) {
  u_int16_t	*list;
  int		loop;

/*
  fprintf(stderr, "total %i  used %i\n",
          temp.size, temp.current - temp.start); 
  fprintf(stderr, "tlb %i\n", c * sizeof(u_int16_t) );
*/

  list = ( u_int16_t * ) temp.current;

/*
No need to advance temp.current.
The tag list is discarded before
more data is allocated from temp
*/

  for( loop = 0 ; c > 0 ; c--, loop++)
    list[loop] =  qtag( *(QWORD *)(cr + loop) );
  return  list;
}


TAG  *order_tag ( u_int32_t *n, u_int32_t *r, DWORD *cr, unsigned int b, unsigned int c ) {

  /* unsigned int	l, o; */
  int		loop;
  u_int16_t	t;
  u_int16_t	*tag;
  TAG		*tags;
  /* DWORD		crc; */

#if __GNUC__ >= 4
  auto int  compare_tag (const void *v0, const void *v1)  {
#else
  static int  compare_tag (const void *v0, const void *v1)  {
#endif
    return  tag[*(u_int32_t *)v0] - tag[*(u_int32_t *)v1];
  }


#if __GNUC__ >= 4
  auto int  compare_crc (const void *v0, const void *v1)  {
#else
  static int  compare_crc (const void *v0, const void *v1)  {
#endif
    u_int32_t	 p0,  p1;
    QWORD        q0,  q1;

    p0  =  *(u_int32_t *)v0;
    p1  =  *(u_int32_t *)v1;

    q0  =  *(QWORD *)(cr + p0);
    q1  =  *(QWORD *)(cr + p1);

         if  ( q0.qword == q1.qword )  return  p0 - p1;
    else if  ( q0.qword >  q1.qword )  return   1;
    else                               return  -1;

  }

  tags           =  ( TAG * ) temp.current;
  temp.current  +=  0x10000 * sizeof(TAG);
  tag            =  tag_list(cr, c);

  qsort(r, b, sizeof(u_int32_t), compare_tag);

  for ( loop = 0; loop < 0x10000 ; loop++)
    tags[loop].index =
    tags[loop].range = 0x0000;

  for ( loop = b - 1 ; loop >= 0 ; loop--) {
          t = tag[ r[loop] ];
    tags[ t ].range++;
    tags[ t ].index = loop;
  }

/*
  free ( tag );
*/

  for( loop = 0; loop < 0x10000; loop++ )
    qsort( &r[tags[loop].index],
              tags[loop].range,
           sizeof(u_int32_t),
           compare_crc);

  return  tags;

}


void make_index(INDEX *r, unsigned char *b, int s) {

  /* int    loop, size, o, t; */

  r->natural     =  natural_block_list  (b, s, &r->naturals);
  r->crc         =  crc_list            (b, r->natural, r->naturals);
  r->ordered     =      list_sig        (r->natural, r->naturals, &r->ordereds);
  r->tags        =  order_tag           (r->natural, r->ordered, r->crc, r->ordereds, r->naturals);

/*
  for    ( loop  =  0;  loop  <  0x10000;              loop++ )
    fprintf(stderr, "tag = %i range = %i\n", loop, r->tags[loop].range);

  for    ( loop  =  0;  loop  <  0x10000;              loop++ )
    for  ( o     =  0;  o     <  r->tags[loop].range;  o++ ) {
      if ( loop != crc_tag ( r->crc[r->ordered[ r->tags[loop].index + o]] ) )
        fprintf(stderr, "Ordering is faulty\n" );
      if ( ( o > 0 ) &&
           ( r->crc[r->ordered[ r->tags[loop].index + o - 1]].dword >
             r->crc[r->ordered[ r->tags[loop].index + o]].dword
         ) ) fprintf(stderr, "Ordering is faulty 2\n");
    }
  fprintf(stderr, "Done checking order\n");
*/

}

#endif

#ifdef SDELTA_3

void make_index(INDEX *r, unsigned char *b, int s) {
  r->natural     =  natural_block_list  (b, s,         &r->naturals);
                    order_blocks        (b, r->natural, r->naturals);
}

#endif

/*

This code, blocks.c written and copyrighted by Kyle Sallee,
creates and orders lists of dynamically sized blocks of data.
It is tuned to creating blocks between 
1 and 127 bytes long that begin after
line feeds and end with line feeds.
Please read LICENSE if you have not already

*/



#include <sys/types.h>
#include <stdio.h>

#include "blocks.h"


LINE	*natural_block_list(unsigned char *b, int s, int *c) {
  LINE			*r, *t;
  unsigned char		*p, *a, *max;
  u_int32_t		i;
  u_int32_t		count;

  t    =  r    =  (LINE *) malloc(s + 256);
  max  =  b + s;

  for ( p = b ; p < max ; t++) {
    t->offset   =  p - b;
    a           =  p;
    for ( count = 1; *p++ != 0x0a ; count++ );

    if ( count > 0x7f ) { p = a + 0x7f; count = 0x7f; }

    t->crc.dword  =  adler32( a , count );
  }

  t--;
  count         =  (u_int32_t)( max - a );
  t->crc.dword  =  adler32 ( a , count );
  t++;

  t->offset     =  s;
  t->crc.dword  =  0;
  i             =  ( t - r );
  *c            =  i++;
  r             =  (LINE *) realloc(r, i * sizeof(LINE));

  return  r;
}

extern int lazy;

unsigned int  *order_tag_crc_offset ( LINE *n, unsigned int c, unsigned int *b ) {

  u_int32_t    *r;
  unsigned int	l;
  unsigned int  t;
  u_int32_t     c0, c1;

  static int  compare_block (const void *v0, const void *v1)  {
    LINE	*p0, *p1;
    int		 diff;

    p0  =  (LINE *)( n + *(unsigned int *)v0 );
    p1  =  (LINE *)( n + *(unsigned int *)v1 );

    diff  =  crc_tag ( p0->crc )  -  crc_tag ( p1->crc );

    if ( diff == 0 )  {
      c0  =  p0->crc.dword;
      c1  =  p1->crc.dword;
            if  ( c0 > c1 )  diff =  1;
      else  if  ( c0 < c1 )  diff = -1;
      else  if  ( p0->offset > p1->offset )  diff =  1;
      else                                   diff = -1;
    }

    return  diff;
  }

  r   =  (u_int32_t *)  malloc (    c * sizeof(u_int32_t) );

  for ( l = t = 0; l < c; l++ )
    if  ( n[l+1].offset - n[l].offset > lazy )  r[t++]  =  l;

  r   =  (u_int32_t *) realloc ( r, t * sizeof(u_int32_t) );
  *b  =  t;

  qsort(r, t, sizeof(u_int32_t), compare_block);
  return  r;

}


INDEX  *make_index(INDEX *r, unsigned char *b, int s) {

  int    loop, size;

  r->natural  =  natural_block_list    (b, s,      &r->naturals);
  r->ordered  =  order_tag_crc_offset  (r->natural, r->naturals, &r->ordereds);
  r->tags     =  malloc ( 0x10000 * sizeof( u_int32_t) );

  for ( loop = 0; loop < 0xffff ; )  r->tags[loop++] = 0xffffffff;

  loop  =  r->ordereds - 1;
  while ( loop >= 0 )
    r->tags[ crc_tag ( r->natural[ r->ordered[loop]].crc ) ]  =  loop--;

  return  r;
}

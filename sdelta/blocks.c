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

u_int32_t	*natural_block_list(unsigned char *b, int s, int *c) {
  u_int32_t		*r, *t;
  unsigned char		*p, *a, *max;
  u_int32_t		i;
  u_int32_t		count;

  t    =  r    =  (u_int32_t *) malloc(s + 64);
  max  =  b + s;

  for ( p = b ; p < max ; t++) {
    *t  =  p - b;
    a   =  p;
    for (; *p++ != 0x0a  &&  p < max ;);
    if  ( ( p - a ) > 0x7f )  { p = a + 0x7f;  }
  }

  *t  =  s;
  i   =  ( t - r );
  *c  =  i++;
  r   =  (u_int32_t *) realloc(r, i * sizeof(u_int32_t));

  return  r;
}


extern int lazy;


DWORD	*crc_list ( unsigned char *b, u_int32_t *n, int c) {
  DWORD  *l, *list;
  int     size;

  l = list = ( DWORD *) malloc( c * sizeof(DWORD) );

  while ( c > 0 )  {
    size = n[1] - n[0];
    if  ( size > 4 )  l->dword = adler32 ( b + *n, size );
    else              l->dword = 0xfff1fff1;

    l++;
    n++;
    c--;
  }
  return  list;
}


DWORD	*crc_list_sig ( unsigned char *b, u_int32_t *n, int c, int *s) {
  DWORD  *l, *list;
  int     size, i, x;

  l = list = ( DWORD *) malloc( c * sizeof(DWORD) );

  x = c;
  i = 0;
  while ( x > 0 )  {
    size = n[1] - n[0];
    if  ( size > 4 )  l->dword = adler32 ( b + *n, size );
    else           {  l->dword = 0xfff1fff1;  i++;  }

    l++;
    n++;
    x--;
  }
  *s = c - i;
  return  list;
}


unsigned int  *order_tag_crc_offset ( u_int32_t *bl, DWORD *cr, unsigned int c, unsigned int b ) {

  u_int32_t    *r;
  u_int32_t	l, t;

  static int  compare_block (const void *v0, const void *v1)  {
    u_int32_t	 p0,  p1;
    DWORD        c0,  c1;
    int          diff;

    c0  =  cr[ *(u_int32_t *)v0 ];
    c1  =  cr[ *(u_int32_t *)v1 ];

    diff  =  crc_tag ( c0 )  -  crc_tag ( c1 );

    if ( diff == 0 )  {
            if  ( c0.dword > c1.dword )  diff =  1;
      else  if  ( c0.dword < c1.dword )  diff = -1;
      else  {
        if  ( *( u_int32_t *)v0 > *( u_int32_t *)v1 )  diff =  1;
        else                                           diff = -1;
      }
    }

    return  diff;
  }

  r   =  (u_int32_t *)  malloc ( b * sizeof(u_int32_t) );

  for ( l = t = 0; t < b; l++ )
    if  ( cr[l].dword != 0xfff1fff1 )  r[t++] = l;

  qsort(r, t, sizeof(u_int32_t), compare_block);
  return  r;

}


/*
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
*/

void make_index(INDEX *r, unsigned char *b, int s) {

  int    loop, size, o;

  r->natural  =  natural_block_list    (b, s, &r->naturals);
  r->crc      =  crc_list_sig          (b, r->natural, r->naturals, &r->ordereds);
  r->ordered  =  order_tag_crc_offset  (r->natural, r->crc, r->naturals, r->ordereds);
  r->tags     =  malloc ( 0x10000 * sizeof( u_int32_t) );

  for ( loop = 0; loop < 0xffff ; )  r->tags[loop++] = 0xffffffff;

  loop  =  r->ordereds - 1;
  while ( loop >= 0 )
    r->tags[ crc_tag ( r->crc[ r->ordered[loop] ] ) ]  =  loop--;

}

/*

This code, blocks.c written and copyrighted by Kyle Sallee,
creates and orders lists of dynamically sized blocks of data.
It is tuned to creating blocks between 
1 and 127 bytes long that begin after
line feeds and end with line feeds.
Please read LICENSE if you have not already

*/



#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>

#include "blocks.h"

u_int32_t	*natural_block_list(unsigned char *b, int s, int *c) {
  u_int32_t		*r, *t;
  unsigned char		*p, *a, *max, *maxp;
  u_int32_t		i;
  u_int32_t		count;

  t    =  r    =  (u_int32_t *) malloc(s + 64);
  max  =  b + s;

  for ( p = b ; p < max ; t++) {
    *t  =  p - b;
    for (maxp = (a = p) + MIN(0x7f, max - p); *p++ != 0x0a && p < maxp;);
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


unsigned int *list_sig ( u_int32_t *bl, DWORD *cr, unsigned int b ) {

  u_int32_t	*r;
  u_int32_t	l, t;

  r     =  (u_int32_t *)  malloc ( b * sizeof(u_int32_t) );

  for ( l = t = 0; t < b; l++ )
    if  ( cr[l].dword != 0xfff1fff1 )  r[t++] = l;

  return  r;
}


TAG  *order_tag ( u_int32_t *r, DWORD *cr, unsigned int b, unsigned int c ) {

  unsigned int	l, o;
  int		loop;
  u_int16_t	t;
  u_int16_t	*tag;
  TAG		*tags;
  DWORD		crc;

  static int  compare_tag (const void *v0, const void *v1)  {
    return  tag[*(u_int32_t *)v0] - tag[*(u_int32_t *)v1];
  }


  static int  compare_crc (const void *v0, const void *v1)  {
    u_int32_t	 p0,  p1;
    DWORD        c0,  c1;
    int          diff;

    p0  =  *(u_int32_t *)v0;
    p1  =  *(u_int32_t *)v1;

    c0  =  cr[ p0 ];
    c1  =  cr[ p1 ];
    if  ( c0.dword == c1.dword ) {
      c0  =  cr[ p0 + 1];
      c1  =  cr[ p1 + 1];
    }

         if  ( c0.dword == c1.dword )  return  p0 - p1;
    else if  ( c0.dword >  c1.dword )  return   1;
    else                               return  -1;

  }

  tags  =                 malloc ( 0x10000 * sizeof(TAG) );
  tag   =  (u_int16_t *)  malloc ( c * sizeof(u_int16_t) );

  for ( l = 0; l < b; l++ ) {
    o   =  r[l];
    crc = cr[o];
    if  ( crc.dword != 0xfff1fff1 )
      tag[o] = crc_tag ( crc );
  }

  qsort(r, b, sizeof(u_int32_t), compare_tag);

  for ( loop = 0; loop < 0xffff ; )
    tags[loop++].range = 0x0;

  loop  =  b - 1;
  while ( loop >= 0 )  {
          t = tag[ r[loop] ];
    tags[ t ].range++;
    tags[ t ].index = loop--;
  }

  free ( tag );

  for( loop = 0; loop < 0x10000; loop++ )
    qsort( &r[tags[loop].index],
              tags[loop].range,
           sizeof(u_int32_t),
           compare_crc);

  return  tags;

}


void make_index(INDEX *r, unsigned char *b, int s) {

  int    loop, size, o, t;

  r->natural     =  natural_block_list  (b, s, &r->naturals);
  r->crc         =  crc_list_sig        (b, r->natural, r->naturals, &r->ordereds);
  r->ordered     =      list_sig        (r->natural, r->crc, r->ordereds);
  r->tags        =  order_tag           (r->ordered, r->crc, r->ordereds, r->naturals);

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

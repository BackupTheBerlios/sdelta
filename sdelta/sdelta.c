/*

sdelta.c was written and copyrighted by Kyle Sallee
You may use it in accordance with the 
Sorcerer Public License version 1.1
Please read LICENSE

sdelta can identify and combine the difference between two files.
The difference, also called a delta, can be saved to a file.
Then, the second of two files can be generated 
from both the delta and first source file.

sdelta is a line blocking dictionary compressor.

*/


#define _GNU_SOURCE

/* for stdin stdout and stderr */
#include <stdio.h>
#include <errno.h>
/* for memcmp */
#include <string.h>
#include <mhash.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/* #include "mmap.h" */
#include "input.h"
#include "sdelta.h"


char	magic[]    =  { 0x13, 0x04, 00, 00 };
int	verbosity  =  0;
int	lazy	   =  4;

void	output_sdelta(FOUND found, TO to, FROM from) {

  u_int32_t		size, origin;
  unsigned char		byte_val;
  int			block;
  int			allocated=0x100000;
  static DWORD		*dwp;
  unsigned int		  offset_unmatched_size;
  u_int32_t		stretch, unmatched_size;
  MHASH			td;

  found.buffer   =  malloc ( allocated );
  memcpy( found.buffer,      &magic,     4  );
  memcpy( found.buffer + 24, &from.sha1, 20 );
  found.offset   =  44;

  dwp = (DWORD *)&to.size;
  found.buffer[found.offset++] = dwp->byte.b3;
  found.buffer[found.offset++] = dwp->byte.b2;
  found.buffer[found.offset++] = dwp->byte.b1;
  found.buffer[found.offset++] = dwp->byte.b0;
  
  stretch   =  0;
  to.block  =  0;

  for ( block = 0;  block < found.count ; block++ ) {

    if  ( ( found.offset + 0x08 ) > allocated ) {
      allocated    +=  0x10000;
      found.buffer  =  (unsigned char *) realloc(found.buffer, allocated);
    }

    stretch   =  found.pair[block].to - to.block;
    to.block  =  found.pair[block].to + found.pair[block].count;

    /*
    printf("to.index.natural[found.pair[block].to].offset  %i\n",
            to.index.natural[found.pair[block].to].offset );
    printf("block                     %i\n", block);
    printf("allocated                 %i\n", allocated);
    printf("found.pair[block].count   %i\n", found.pair[block].count);
    printf("found.pair[block].from    %i\n", found.pair[block].from);
    printf("found.pair[block].to      %i\n", found.pair[block].to);
    printf("stretch                   %i\n", stretch);
    printf("\n");
    */

    /*
    printf("\n");
    printf("found.pair[block].to + size %i\n", found.pair[block].to + size);
    printf("to.index.natural[found.pair[block].to + size].offset %i\n",
            to.index.natural[found.pair[block].to + size].offset);
    printf("\n");
    */

    origin = found.pair[block].from;
    size   = found.pair[block].count;

                                   byte_val  = 0x00;
    if ( origin     >= 0x1000000 ) byte_val |= 0xc0;  else
    if ( origin     >= 0x10000   ) byte_val |= 0x80;  else
    if ( origin     >= 0x100     ) byte_val |= 0x40;

    if ( size       >= 0x1000000 ) byte_val |= 0x30;  else
    if ( size       >= 0x10000   ) byte_val |= 0x20;  else
    if ( size       >= 0x100     ) byte_val |= 0x10;

    if   ( stretch  > 0 )  {       byte_val |= 0x02;
      if ( stretch  >= 0x1000000 ) byte_val |= 0x0c;  else
      if ( stretch  >= 0x10000   ) byte_val |= 0x08;  else
      if ( stretch  >= 0x100     ) byte_val |= 0x04;
    }

    found.buffer[found.offset++] =  byte_val;

    if ( verbosity > 1 )
      fprintf(stderr, "block %i  control %x  stretch %i  to %i  count %i  from %i  to.offset %i\n",
              block, byte_val, stretch,
              found.pair[block].to,
              found.pair[block].count,
              found.pair[block].from,
              to.index.natural[found.pair[block].to]);

    dwp = (DWORD *)&origin;
    if ( origin      >=  0x1000000 ) found.buffer[found.offset++] = dwp->byte.b3;
    if ( origin      >=  0x10000   ) found.buffer[found.offset++] = dwp->byte.b2;
    if ( origin      >=  0x100     ) found.buffer[found.offset++] = dwp->byte.b1;
                                     found.buffer[found.offset++] = dwp->byte.b0;

    dwp = (DWORD *)&size;
    if ( size        >=  0x1000000 ) found.buffer[found.offset++] = dwp->byte.b3;
    if ( size        >=  0x10000   ) found.buffer[found.offset++] = dwp->byte.b2;
    if ( size        >=  0x100     ) found.buffer[found.offset++] = dwp->byte.b1;
                                     found.buffer[found.offset++] = dwp->byte.b0;

    dwp = (DWORD *)&stretch;
    if ( stretch > 0 ) {
      if ( stretch   >=  0x1000000 ) found.buffer[found.offset++] = dwp->byte.b3;
      if ( stretch   >=  0x10000   ) found.buffer[found.offset++] = dwp->byte.b2;
      if ( stretch   >=  0x100     ) found.buffer[found.offset++] = dwp->byte.b1;
                                     found.buffer[found.offset++] = dwp->byte.b0;
    }
  }

         unmatched_size   =  0;
  offset_unmatched_size   =  found.offset;
  found.offset           +=  4;
     to.offset            =  0;

  for ( block = 0; block < found.count ; block++ ) {
    stretch          =  to.index.natural[found.pair[block].to]  -  
                        to.offset;

    if  ( stretch > 0 ) {
      if  ( ( found.offset + stretch ) > allocated ) {
        allocated    +=      stretch + 0x10000;
        found.buffer  =  (unsigned char *) realloc(found.buffer, allocated);
      }

      memcpy ( found.buffer + found.offset,
                  to.buffer +    to.offset, stretch );

      unmatched_size  += stretch;
      found.offset    += stretch;
    }
    to.offset     = to.index.natural[found.pair[block].to +
                                     found.pair[block].count ];
  }

  dwp = (DWORD *)&unmatched_size;
  found.buffer[offset_unmatched_size++] = dwp->byte.b3;
  found.buffer[offset_unmatched_size++] = dwp->byte.b2;
  found.buffer[offset_unmatched_size++] = dwp->byte.b1;
  found.buffer[offset_unmatched_size++] = dwp->byte.b0;
  
  td  =  mhash_init	( MHASH_SHA1 );
         mhash		( td, found.buffer + 24, found.offset - 24 );
         mhash_deinit	( td, found.buffer +  4);

  fwrite( found.buffer, 1, found.offset, stdout );
  free(found.buffer);

}


#define  leap(frog) \
  if ( start >= frog ) { \
    from.block   =   from.index.ordered[where + frog]; \
    fcrc0.dword  =   from.index.crc[from.block].dword; \
    if ( crc0.dword  >  fcrc0.dword  ) { \
      where  +=  frog; \
      start  -=  frog; \
    } \
    else if  ( crc0.dword  ==  fcrc0.dword ) { \
      fcrc1.dword = from.index.crc[from.block+1].dword; \
      if ( crc1.dword  >  fcrc1.dword ) { \
            where  +=  frog; \
            start  -=  frog; \
      } \
      else  start   =  frog - 1; \
    } else  start   =  frog - 1; \
  }


#define leap_0x4()     leap(0x4)
#define leap_0x8()     leap(0x8)      leap_0x4
#define leap_0x20()    leap(0x20)     leap_0x10
#define leap_0x40()    leap(0x40)     leap_0x20
#define leap_0x80()    leap(0x80)     leap_0x40
#define leap_0x100()   leap(0x100)    leap_0x80
#define leap_0x200()   leap(0x200)    leap_0x100
#define leap_0x400()   leap(0x400)    leap_0x200
#define leap_0x800()   leap(0x800)    leap_0x400
#define leap_0x1000()  leap(0x1000)   leap_0x800
#define leap_0x2000()  leap(0x2000)   leap_0x1000
#define leap_0x4000()  leap(0x4000)   leap_0x2000
#define leap_0x8000()  leap(0x8000)   leap_0x4000
#define leap_0x10000() leap(0x10000)  leap_0x8000


void  make_sdelta(INPUT_BUF *from_ibuf, INPUT_BUF *to_ibuf)  {
  MHASH			td;
  FROM			from;
  TO			to;
  MATCH			match;
  FOUND			found;
  unsigned int		count, line, size, total, where, start, limit;
  u_int16_t		tag;
  DWORD			crc0, crc1, fcrc0, fcrc1;
  pthread_t		from_thread, to_thread;


void  *prepare_from(void *nothing)  {
  td  =  mhash_init	( MHASH_SHA1 );
         mhash		( td, from.buffer, from.size );
         mhash_deinit	( td, from.sha1 );

  make_index  ( &from.index, from.buffer, from.size );
  MADVISE_IBUF( from_ibuf,   from.buffer, from.size, MADV_RANDOM );
  return NULL;
}

void  *prepare_to(void *nothing)  {
  to.index.natural  =  natural_block_list ( to.buffer, to.size, &to.index.naturals );
  to.index.crc      =  crc_list ( to.buffer, to.index.natural, to.index.naturals );
  found.pair        =  malloc ( sizeof(PAIR) * to.index.naturals );
  return NULL;
}
/*
  from.buffer = b1;
  to.buffer   = b2;
  from.size   = s1;
  to.size     = s2;
  */
  from.buffer = from_ibuf->buf;
  to.buffer   = to_ibuf->buf;
  from.size   = from_ibuf->size;
  to.size     = to_ibuf->size;

  pthread_create( &from_thread, NULL, prepare_from, NULL );
  pthread_create(   &to_thread, NULL, prepare_to,   NULL );
  pthread_join  (  from_thread, NULL );
  pthread_join  (    to_thread, NULL );


/*
  prepare_from(NULL);
  prepare_to(NULL);
*/

  found.count       =  0;
  to.block          =  0;

  while  ( to.index.naturals  >  to.block )  {

    if   (              to.index.crc[to.block    ].dword != 0xfff1fff1 )  {
      crc0.dword     =  to.index.crc[to.block    ].dword;
      crc1.dword     =  to.index.crc[to.block + 1].dword;
      tag            =  crc_tag ( crc0 );
      start          =  from.index.tags[tag].range;

      if  ( start == 0 )  {  to.block++;  continue;  }

      start--;
      where  =  from.index.tags[tag].index;

      while ( start >= 0x10000 )
      leap(0x10000);
      leap(0x8000);
      leap(0x4000);
      leap(0x2000);
      leap(0x1000);
      leap(0x800);
      leap(0x400);
      leap(0x200);
      leap(0x100);
      leap(0x80);
      leap(0x40);
      leap(0x20);
      leap(0x10);
      leap(0x8);
      leap(0x4);

      match.count    =  1;
         to.limit    =  to.index.naturals - to.block;

      while ( ( where        <   from.index.ordereds               ) &&
              ( from.block   =   from.index.ordered[where]         ) &&
              ( fcrc0.dword  =   from.index.crc[from.block].dword  ) &&
              ( tag          ==  crc_tag(fcrc0)                    ) &&
              (  crc0.dword  >   fcrc0.dword                       )
            )  where++;

      fcrc1.dword  =  crc1.dword + 1;

      while ( ( where        <   from.index.ordereds               ) &&
              ( from.block   =   from.index.ordered[where]         ) &&
              ( fcrc0.dword  =   from.index.crc[from.block].dword  ) &&
              (  crc0.dword  ==  fcrc0.dword                       ) &&
              ( fcrc1.dword  =   from.index.crc[from.block+1].dword  ) &&
              (  crc1.dword  >   fcrc1.dword                       )
            )  where++;

      if ( ( crc0.dword == fcrc0.dword ) &&
           ( crc1.dword == fcrc1.dword )
         ) {

        while ( ( where        <   from.index.ordereds                 ) &&
                ( from.block   =   from.index.ordered[where]           ) &&
                ( fcrc1.dword  =   from.index.crc[from.block+1].dword  ) &&
                ( crc1.dword   ==  fcrc1.dword                         )
              ) {

          count       =  2;
          from.limit  =  from.index.naturals - from.block;

          if ( to.limit > from.limit )
            limit  =      from.limit;  else
            limit  =        to.limit;

          while  ( ( count < limit )  &&
                   ( from.index.crc[from.block + count ].dword ==
                       to.index.crc[  to.block + count ].dword ) )
            count++;

          while ( count > match.count ) {
            if ( memcmp( from.index.natural[from.block] + from.buffer,
                          to.index.natural[   to.block] +   to.buffer,
                          to.index.natural[   to.block + count] -
                          to.index.natural[   to.block        ]
                         ) == 0 ) {
              match.line   =  from.block;
              match.count  =  count;
              count  =  0;
            } else  count--;
          }
          where++;
        }
      }  /* finished finding matches for to.block */

      if ( match.count > 1 ) {
        found.pair[found.count].to      =     to.block;
        found.pair[found.count].from    =  match.line;
        found.pair[found.count].count   =  match.count;
        to.block                       +=  match.count;
        found.count++;
      } else  to.block++;
    } else  to.block++;
  }

  found.pair[found.count  ].to     =  to.index.naturals;
  found.pair[found.count  ].from   =  0;
  found.pair[found.count++].count  =  0;
  found.pair                       =  realloc ( found.pair, sizeof(PAIR) * found.count );

  if ( verbosity > 0 ) {
    fprintf(stderr, "Statistics for sdelta generation.\n");

    fprintf(stderr, "Blocks in from                      %i\n", from.index.naturals);
    fprintf(stderr, "Blocks in to                        %i\n",   to.index.naturals);
    fprintf(stderr, "Sorted blocks in from               %i\n", from.index.ordereds);

    total=0;
    for ( where = 0; where < found.count; where++)
      total += found.pair[where].count;
    fprintf(stderr, "Matching blocks                     %i\n", total);
    fprintf(stderr, "Umatched blocks                     %i\n", to.index.naturals - total);

    fprintf(stderr, "Matching blocks sequences           %i\n", found.count);

    total=0;
    for ( where = 0; where < found.count; where++)
      if (found.pair[where].count > 1 )  total++;
    fprintf(stderr, "Matches with consecutive sequences  %i\n", total);

    total=0;
    for ( where = 0; where < found.count - 1; where++)
      if  ( ( found.pair[where].to +
              found.pair[where].count )  ==
              found.pair[where+1].to  )
        total++;
    fprintf(stderr, "Adjacent matching blocks sequences  %i\n", total);

    total=0;
    for ( where = 0; where < found.count; where++) {
      line = found.pair[where].to; 
      for ( count = 0 ; count < found.pair[where].count ; count++) {
       total += to.index.natural[line + 1] -
                to.index.natural[line    ];
       line++;
      }
    }

    fprintf(stderr, "Bytes in to                         %i\n", to.size);
    fprintf(stderr, "Bytes matched                       %i\n", total);
    fprintf(stderr, "Bytes unmatched                     %i\n", to.size - total);
  }

  free   (  from.index.natural     );
  free   (  from.index.crc         );
  free   (  from.index.ordered     );
  /* munmap (  from.buffer, from.size ); */
  unload_buf(from_ibuf);

  output_sdelta(found, to, from);

  /* munmap (    to.buffer,   to.size ); */
  unload_buf(to_ibuf);
  free   (    to.index.natural     );
  free   (    to.index.crc         );
  free   ( found.pair              );

}



void   make_to(INPUT_BUF *from_ibuf, INPUT_BUF *found_ibuf)  {
  FOUND			found;
  static FROM		from, delta;
  TO			to;
  static DWORD		*dwp;
  unsigned char		control;
  u_int32_t		stretch, line;
  u_int32_t		block;
  u_int32_t		size;
  MHASH			td;

  if (from_ibuf) {
      from.buffer  =  from_ibuf->buf;
      from.size    =  from_ibuf->size;
  }
  
  found.buffer  =  found_ibuf->buf;
  found.size    =  found_ibuf->size;

  if  ( memcmp(found.buffer, &magic, 4) != 0 ) {
    fprintf(stderr, "Input on stdin did not start with sdelta magic.\n");
    fprintf(stderr, "Hint: cat sdelta_file from_file | sdelta  > to_file\n");
    exit(EXIT_FAILURE);
  }

  found.offset       =  44;  /* Skip the magic and 2 sha1 */
  dwp                =  (DWORD *)&to.size;
  dwp->byte.b3       =  found.buffer[found.offset++];
  dwp->byte.b2       =  found.buffer[found.offset++];
  dwp->byte.b1       =  found.buffer[found.offset++];
  dwp->byte.b0       =  found.buffer[found.offset++];
  found.pair         =  malloc ( found.size );
  found.count        =  0;
  line               =  0;

  size          =  1;
  while ( size !=  0 ) {

    control     =  found.buffer[found.offset++];
    dwp         =  (DWORD *)&found.pair[found.count].from;
    dwp->dword  =  0;

    switch ( control & 0xc0 ) {
      case 0xc0 : dwp->byte.b3 = found.buffer[found.offset++];
                  dwp->byte.b2 = found.buffer[found.offset++];
                  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      case 0x80:  dwp->byte.b2 = found.buffer[found.offset++];
                  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      case 0x40:  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      default:    dwp->byte.b0 = found.buffer[found.offset++];
    }

    dwp         =  (DWORD *)&found.pair[found.count].count;
    dwp->dword  =  0;
    switch ( control & 0x30 ) {
      case 0x30 : dwp->byte.b3 = found.buffer[found.offset++];
                  dwp->byte.b2 = found.buffer[found.offset++];
                  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      case 0x20:  dwp->byte.b2 = found.buffer[found.offset++];
                  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      case 0x10:  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      default:    dwp->byte.b0 = found.buffer[found.offset++];
    }
    size        =  found.pair[found.count].count;
    dwp         =  (DWORD *)&stretch;
    dwp->dword  =  0;

    if  ( ( control & 2 )  == 2 ) {
    switch ( control & 0x0c ) {
      case 0x0c : dwp->byte.b3 = found.buffer[found.offset++];
                  dwp->byte.b2 = found.buffer[found.offset++];
                  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      case 0x08:  dwp->byte.b2 = found.buffer[found.offset++];
                  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      case 0x04:  dwp->byte.b1 = found.buffer[found.offset++];
                  dwp->byte.b0 = found.buffer[found.offset++];
                  break;
      default:    dwp->byte.b0 = found.buffer[found.offset++];
    }
    line += stretch;
    }

    if ( verbosity > 1 )
      fprintf(stderr, "block %i  control %x  stretch %i  to %i  count %i  from %i\n",
              found.count,
              control,
              stretch,
              line,
              found.pair[found.count].count,
              found.pair[found.count].from);

            found.pair[found.count  ].to  = line;
    line += found.pair[found.count++].count;

  };

  found.pair    =  realloc( found.pair, sizeof(PAIR) * found.count );

  dwp           =  (DWORD *)&delta.size;
  dwp->byte.b3  =  found.buffer[found.offset++];
  dwp->byte.b2  =  found.buffer[found.offset++];
  dwp->byte.b1  =  found.buffer[found.offset++];
  dwp->byte.b0  =  found.buffer[found.offset++];

  delta.buffer  =  found.buffer + found.offset;
  if  ( from.buffer == NULL )  {
           from.buffer  =  found.buffer + found.offset + delta.size;
           from.size    =  found.size   - found.offset - delta.size;
          found.size    =  found.size   - from.size    - 24;
  } else  found.size   -=  24;


  td  =  mhash_init     ( MHASH_SHA1 );
         mhash          ( td, from.buffer, from.size );
         mhash_deinit   ( td, from.sha1 );

  td  =  mhash_init     ( MHASH_SHA1 );
         mhash          ( td, found.buffer + 24, found.size);
         mhash_deinit   ( td, found.sha1 );

  if  ( memcmp( found.sha1, found.buffer + 4, 20 ) != 0 ) {
    fprintf(stderr, "The sha1 for this sdelta did not match.\nAborting.\n");
    exit(EXIT_FAILURE);
  }

  if  ( memcmp( from.sha1, found.buffer + 24, 20 ) != 0 ) {
    fprintf(stderr, "The sha1 for the dictionary file did not match.\nAborting.\n");
    exit(EXIT_FAILURE);
  }

   from.index.natural  =  natural_block_list (  from.buffer,  from.size,  &from.index.naturals );
  delta.index.natural  =  natural_block_list ( delta.buffer, delta.size, &delta.index.naturals );

  delta.offset  =  0;
   from.offset  =  0;
     to.offset  =  0;
   from.offset  =  0;
     to.block   =  0;
  delta.block   =  0;
  to.buffer     =  malloc ( to.size );

  for ( block = 0; block < found.count; block++ ) {
    stretch = found.pair[block].to - to.block;
    if ( stretch > 0 ) {
      size  = delta.index.natural[delta.block + stretch] -
              delta.index.natural[delta.block          ];
      if ( verbosity > 1 ) {
        fprintf(stderr, "Writing block %i  stretch %i  size %i\n",
                                 block,    stretch,    size);
        fprintf(stderr, "to.block %i  to.offset %i  delta.offset %i\n",
                         to.block,    to.offset,    delta.offset);
      }
      memcpy( to.buffer + to.offset, delta.buffer + delta.offset, size );
         to.offset += size;
      delta.offset += size;
      delta.block  += stretch;
         to.block  += stretch;
    }

    size = from.index.natural[found.pair[block].from + 
                              found.pair[block].count ] -
           from.index.natural[found.pair[block].from  ];
    from.offset = from.index.natural[ found.pair[block].from ];
    if ( verbosity > 1 ) {
      fprintf(stderr, "Writing block %i  blocks %i  size %i  from %i\n",
                               block,    found.pair[block].count, size, found.pair[block].from);
      fprintf(stderr, "to block %i  to.offset %i  from.offset %i\n\n",
                       to.block,    to.offset,    from.offset);
    }
    memcpy( to.buffer + to.offset, from.buffer + from.offset, size );
    to.offset += size;
    to.block  += found.pair[block].count;
  }

  fwrite( to.buffer, 1, to.offset, stdout );

  if (from_ibuf)
      unload_buf(from_ibuf);
  unload_buf(found_ibuf);
}



void  help(void)  {
  printf("\nsdelta designed programmed and copyrighted by\n");
  printf("Kyle Sallee in 2004, All Rights Reserved.\n");
  printf("sdelta is distributed under the Sorcerer Public License version 1.1\n");
  printf("Please read /usr/doc/sdelta/LICENSE\n\n");

  printf("sdelta records the differences between source tarballs.\n");
  printf("First, sdelta can make a delta patch between two files.\n");
  printf("Then,  sdelta can make the second file when given both\n");
  printf("the previously generated sdelta and the first file.\n\n");

  printf("Below is an example to make a bzip2 compressed sdelta patch file.\n\n");
  printf("$ sdelta linux-2.6.7.tar linux-2.6.8.1.tar | bzip2 -9 > \\\n");
  printf("> linux-2.6.7-linux-2.6.8.1.tar.sdelta.bz2\n\n\n");

  printf("Below is an example for making linux-2.6.8.1.tar\n\n");
  printf("$ ( bzcat linux-2.6.7-linux-2.6.8.1.tar.sdelta.bz2;\n");
  printf(">     cat linux-2.6.7.tar ) | ./sdelta > linux-2.6.8.1.tar\n\n");
  printf("On FreeBSD it could be signifigantly faster to issue:\n");
  printf("./sdelta linux-2.6.7 linux-2.6.7-linux-2.6.8.1.tar.sdelta > linux-2.6.8.1.tar\n");
  exit(EXIT_FAILURE);
}


/* void  parse_parameters( char *f1, char *f2)  {
  size_t   s1,  s2;
  char    *b1, *b2;

  b1=mmap_file( f1, &s1 );
  b2=mmap_file( f2, &s2 );

  if ( memcmp( b2, &magic, 4 ) == 0 )
        make_to     ( b1, s1, b2, s2 );
  else  make_sdelta ( b1, s1, b2, s2 );
  return;
}

void  parse_stdin(void) {
  size_t	 s;
  char		*b;

  b=mmap_stdin(&s);
  make_to ( NULL, 0, b, s );
  return;
} */

void  parse_parameters( char *f1, char *f2)  {
  static INPUT_BUF b1, b2;

  load_buf(f1, &b1);
  load_buf(f2, &b2);

  if ( memcmp( b2.buf, &magic, 4 ) == 0 )
        make_to     (&b1, &b2);
  else  make_sdelta (&b1, &b2);
}

void  parse_stdin(void) {
  static INPUT_BUF b;

  load_buf(NULL, &b);
  make_to (NULL, &b);
} 

int	main	(int argc, char **argv)  {

  if  ( NULL !=  getenv("SDELTA_VERBOSE") )
    sscanf(      getenv("SDELTA_VERBOSE"), "%i", &verbosity );  

  switch (argc) {
    case  3 :  parse_parameters(argv[1], argv[2]);  break;
    case  1 :  parse_stdin();                       break;
    default :  help();                              break;
  }
  exit(EXIT_SUCCESS);
}

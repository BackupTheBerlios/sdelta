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

#include "buffer.h"
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
              to.index.natural[found.pair[block].to].offset);

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
    stretch          =  to.index.natural[found.pair[block].to].offset  -  
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
                                     found.pair[block].count ].offset;
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

void  make_sdelta(char *fromfilename, char *tofilename)  {
  FILE  		*stream;
  MHASH			td;
  FROM			from;
  TO			to;
  MATCH			match;
  FOUND			found;
  unsigned int		count, line, size, total, where, limit;
  u_int16_t		tag;
  DWORD			crc, fcrc;

  from.buffer = buffer_file ( fromfilename, &from.size );

  if  ( from.size  ==  0    )  {
    perror("Problem reading from file");
    exit(EXIT_FAILURE);
  }

  td  =  mhash_init	( MHASH_SHA1 );
         mhash		( td, from.buffer, from.size );
         mhash_deinit	( td, from.sha1 );

  make_index( &from.index, from.buffer, from.size );

  to.buffer = buffer_file ( tofilename, &to.size );

  if  ( to.size  ==  0    )  {
    perror("Problem reading to file");
    exit(EXIT_FAILURE);
  }

  to.index.natural  =  natural_block_list ( to.buffer, to.size, &to.index.naturals );
  found.pair        =  malloc ( sizeof(PAIR) * to.index.naturals );
  found.count       =  0;
  to.block          =  0;

  while  ( to.index.naturals                >  to.block )  {

    if   ( to.index.natural[to.block+1].offset -
           to.index.natural[to.block  ].offset  >  4)  {
/*         to.index.natural[to.block  ].offset  >  lazy )  {  */

      crc.dword      =  to.index.natural[to.block].crc.dword;
      tag            =  crc_tag ( crc );
      where          =  from.index.tags[tag];
      match.count    =  0;
         to.limit    =  to.index.naturals - to.block;

/*
      while ( ( where  <  from.index.ordereds ) &&
              ( tag    == crc_tag(from.index.natural[from.index.ordered[where]].crc) ) ) {
*/
      while ( ( where       <  from.index.ordereds                      ) &&
              ( from.block  =  from.index.ordered[where]                ) &&
              ( fcrc.dword  =  from.index.natural[from.block].crc.dword ) &&
              ( tag         == crc_tag(fcrc)                            ) &&
              ( crc.dword   >= fcrc.dword                               )
            ) {


        if ( crc.dword == fcrc.dword ) {

          count       =  1;
          from.limit  =  from.index.naturals - from.block;

          if ( to.limit > from.limit )
            limit  =      from.limit;  else
            limit  =        to.limit;

          while  ( ( count < limit )  &&
                   ( from.index.natural[from.block + count ].crc.dword ==
                       to.index.natural[  to.block + count ].crc.dword ) )
            count++;

          while ( count > match.count ) {
            if ( memcmp( from.index.natural[from.block].offset + from.buffer,
                          to.index.natural[   to.block].offset +   to.buffer,
                          to.index.natural[   to.block + count].offset -
                          to.index.natural[   to.block        ].offset
                         ) == 0 ) {
              match.line   =  from.block;
              match.count  =  count;
              count  =  0;
            } else  count--;
          }
        }
        where++;
      }  /* finished finding matches for to.block */

      if ( ( match.count > 0 ) &&
           ( from.index.natural[match.line + match.count + 1].offset -
             from.index.natural[match.line                  ].offset > 50 ) )  {
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
    fprintf(stderr, "Statistics for sdelta from, %s, to, %s.\n", fromfilename, tofilename);

    fprintf(stderr, "Blocks in from                      %i\n", from.index.naturals);
    fprintf(stderr, "Blocks in to                        %i\n",   to.index.naturals);

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
       total += to.index.natural[line + 1].offset -
                to.index.natural[line    ].offset;
       line++;
      }
    }

    fprintf(stderr, "Bytes in to                         %i\n", to.size);
    fprintf(stderr, "Bytes matched                       %i\n", total);
    fprintf(stderr, "Bytes unmatched                     %i\n", to.size - total);

    total=0;
    for ( where = 0; where < found.count - 2; where++) {
      line = found.pair[where].count;
      line = to.index.natural[found.pair[where + 1].to].offset -
             to.index.natural[found.pair[where    ].to + line].offset;

      if  (total < line)  total  =  line;

      if    ( verbosity > 2 )
        if  ( 0         < line )
          fprintf(stderr, "Unmatched gap of size %i at offset %i\n",
                  line, to.index.natural[found.pair[where].to + line].offset);
    }
    fprintf(stderr, "Largest unmatched sequence of bytes %i\n", total);
  }

  free ( from.index.natural );
  free ( from.index.ordered );
  free ( from.buffer        );

  output_sdelta(found, to, from);

  free (   to.buffer        );
  free (   to.index.natural );
  free ( found.pair         );

}


void	make_to(void)  {
  FOUND			found;
  FROM			from, delta;
  TO			to;
  static DWORD		*dwp;
  unsigned char		control;
  u_int32_t		stretch, line;
  u_int32_t		block;
  u_int32_t		size;
  MHASH			td;

  found.buffer  =  buffer_stream(stdin, &found.size);

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
   from.buffer  =  found.buffer + found.offset + delta.size;
   from.size    =  found.size   - found.offset - delta.size;
   found.size   =  found.size   - from.size    - 24;

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
      size  = delta.index.natural[delta.block + stretch].offset -
              delta.index.natural[delta.block          ].offset;
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
                              found.pair[block].count ].offset -
           from.index.natural[found.pair[block].from  ].offset;
    from.offset = from.index.natural[ found.pair[block].from ].offset;
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

  free(   to.buffer);
  free(found.buffer);
}


void  make_to_new(char *dict_filename, char *sdelta_filename)  {
  FOUND			found;
  FROM			from, delta;
  TO			to;
  static DWORD		*dwp;
  unsigned char		control;
  u_int32_t		stretch, line;
  u_int32_t		block;
  u_int32_t		size;
  MHASH			td;

/*  found.buffer  =  buffer_stream(stdin, &found.size); */

  found.buffer = buffer_file( sdelta_filename, &found.size );

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

/*
   from.buffer  =  found.buffer + found.offset + delta.size;
   from.size    =  found.size   - found.offset - delta.size;
   found.size   =  found.size   - from.size    - 24;
*/
   from.buffer  =  buffer_file( dict_filename, &from.size );
   found.size   =  found.size   -  24;


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
      size  = delta.index.natural[delta.block + stretch].offset -
              delta.index.natural[delta.block          ].offset;
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
                              found.pair[block].count ].offset -
           from.index.natural[found.pair[block].from  ].offset;
    from.offset = from.index.natural[ found.pair[block].from ].offset;
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

  free(   to.buffer);
  free(found.buffer);
  free( from.buffer);
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


int  check_magic( char *n )  {

  FILE *s;
  char b[5];

  s = fopen( n, "r" );

  if   ( s  == NULL ) {
    fprintf( stderr, "Problem opening %s\n", n);
    exit(EXIT_FAILURE);
  }

  fread( b, 4, 1, s);
  close(s);
  return  memcmp( b, &magic, 4 );

}


int	main	(int argc, char **argv)  {

  if  ( NULL !=  getenv("SDELTA_VERBOSE") )
    sscanf(      getenv("SDELTA_VERBOSE"), "%i", &verbosity );  

  if  ( NULL !=  getenv("SDELTA_LAZY") ) {
    sscanf(      getenv("SDELTA_LAZY"), "%i", &lazy );
    if ( ( lazy < 4 ) || ( lazy > 40 ) )  lazy=4;
  }

  if  (argc == 3)  {
    if  ( check_magic( argv[2] ) == 0 )  make_to_new( argv[1], argv[2] );
    else                                 make_sdelta( argv[1], argv[2] );
  }

  else  if	(argc == 1)  make_to();
  else                       help();
  exit(EXIT_SUCCESS);
}

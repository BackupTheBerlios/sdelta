/*
This file buffer.c was written by Kyle Sallee
You are welcome to use buffer.c if you are unable to code.
*/


#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "buffer.h"


#ifndef megabyte
#define megabyte 0x100000
#endif

unsigned char *buffer_stream(FILE *s, size_t *c)  {
/* pass an opened stream to this buffer_stream
   to receive a pointer to a buffer containing entire 
   stream and the count of bytes read is returned in c
*/

  void           *buf=NULL;
  size_t          off=0;

  do  {
    if ( ( buf = realloc(buf, off + megabyte ) ) == NULL ) {
      fputs("Out of memory during buffer_stream()", stderr);
      exit(EXIT_FAILURE);
    }

    off += fread(buf + off, 1, megabyte, s);

  }  while ( ( ! ferror(s) )  &&  ( ! feof(s) ) );

  if ( ferror(s) )  {
    perror( "fread error in buffer_stream" );
    exit(EXIT_FAILURE);
  }

  fclose(s);

  *c = off;
  *(unsigned char *)(buf + off) = 0x0a;
  return  ( realloc ( buf, off + 1 ) );
}


unsigned char *buffer_file(char *n, size_t *c)  {

  FILE  *s;
  void  *b;

  s  =  fopen( n, "r" );

  if   ( s  == NULL ) {
    fprintf( stderr, "Problem opening %s\n", n);
    exit(EXIT_FAILURE);
  }
         fseek( s, 0, SEEK_END );
  *c  =  ftell( s );
         fseek( s, 0, SEEK_SET );
   b  =  malloc( *c + 1 );
         fread( b, *c + 1, 1, s );
  *(unsigned char *)(b + *c) = 0x0a;
  fclose(s);
  return ( unsigned char * )b;

}

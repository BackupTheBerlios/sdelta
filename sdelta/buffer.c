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


#ifdef USE_MAP_ANON_FOR_STDIN_INPUT

unsigned char *read_file_mmap_anon(int fd, size_t prealloc, size_t *sz) {
    unsigned char *buf;
    size_t c;

    *sz = 0;

    /* 
       Using zero as the `prealloc' parameter makes sense only on certain
       operating systems (notably FreeBSD), where mmap(), when being used with
       the MAP_ANON flag, doesn't try to actually reserve any memory, thus
       allowing us to request the absolute maximum, which is 0x7FFFFFFF bytes.

       A non-zero prealloc assumes that you know the exact file size.
     */

    prealloc = prealloc ? prealloc + 1 : MAX_MAP_ANON;

    if ((buf = mmap(NULL, prealloc, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
	perror("mmap");
	exit(EXIT_FAILURE);
    }

    while (c = read(fd, buf + *sz, prealloc - *sz)) {
	if (c == -1) {
	    perror("read");
	    munmap(buf, prealloc);
	    exit(EXIT_FAILURE);
	}

	*sz += c;

	if (prealloc == *sz) {
	    /* need one extra byte for the terminating line feed */
	    fprintf(stderr,
		    "read_file_mmap_anon: can't read more than %u bytes, "
		    "increase `prealloc' or use 0\n", prealloc-1);
	    munmap(buf, prealloc);
	    exit(EXIT_FAILURE);
	}
    }

    buf[*sz] = 0x0a;

    return buf;
}

#endif /* USE_MAP_ANON_FOR_STDIN_INPUT */

#include <stdio.h>
#include <stdlib.h>


/*
   if you are getting error messages like: "mmap: Cannot allocate memory",
   comment out the following `#ifdef __FreeBSD__ ... #endif' block,
   or just #undef USE_MAP_ANON_FOR_STDIN_INPUT
 */

#ifdef __FreeBSD__
#include <sys/mman.h>
#ifdef MAP_ANON
#define USE_MAP_ANON_FOR_STDIN_INPUT
#define MAX_MAP_ANON 0x7FFFFFFF
unsigned char	*read_file_mmap_anon	(int, size_t, size_t *);
#endif /* MAP_ANON */
#endif /* __FreeBSD__ */


unsigned char	*buffer_file		(char *, size_t *);
unsigned char	*buffer_stream		(FILE *, size_t *);
unsigned char	*buffer_known_stream	(FILE *, size_t );

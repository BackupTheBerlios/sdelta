
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "mmap.h"


unsigned char *mmap_file(char *n, size_t *c)  {

  struct stat	st;
  size_t	size;
  int		fd;
  void		*b;

  if ( (stat(n, &st)              == -1)  ||
       ((*c = st.st_size)         ==  0)  ||
       ((fd = open(n, O_RDONLY))  == -1)  ||
       ((b = mmap(NULL, *c, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
     )
  {
     fprintf(stderr, "Error mmaping %s\n", n);
     exit(EXIT_FAILURE);
  }

  madvise( b, *c, MADV_SEQUENTIAL );

  close(fd);

  return ( unsigned char * )b;

}


/* This might very well reserve near 2G or 1G of address space?  */

#ifdef __FreeBSD__
#define MAX_MAP_ANON 0x7FFFFFFF
#else
#define MAX_MAP_ANON 0x40000000
#endif

unsigned char *mmap_stdin(size_t *c) {
  unsigned char *b;
  size_t         s;

  if ((b = mmap(NULL, MAX_MAP_ANON, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
    fprintf(stderr, "Problem mmaping %x bytes for stdin\n", MAX_MAP_ANON);
    exit(EXIT_FAILURE);
  }

  *c = 0;
  while (s = read(0, b + *c, MAX_MAP_ANON)) {
    *c += s;
    if  ( s ==  -1 )  {
      perror("Error occured reading from stdin.\n");
      exit(EXIT_FAILURE);
    }
  }
  return  b;
}

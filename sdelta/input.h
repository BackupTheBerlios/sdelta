/* $Id: input.h,v 1.2 2005/01/10 00:16:23 svinn Exp $ */

#ifndef __INPUT_H__
#define __INPUT_H__


#include <sys/types.h>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/user.h>
#else
#include <sys/param.h>
#endif
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* one realloc step */
#define IBUF_ALLOC_QUANT	0x400000

/* mmap is enabled by default */
#define USE_MMAP

#if defined(__linux__) || defined(__FreeBSD__)
#define USE_MADVISE
#define USE_MAP_ANON_STDIN
#define MAX_MAP_ANON		0x40000000
#endif

#ifdef __linux__
#define USE_MREMAP
#endif

#ifdef USE_MADVISE
/* USE_MADVISE implies USE_MMAP */
#ifndef USE_MMAP
#define USE_MMAP
#endif
/* MADVISE_IBUF macro needs this INPUT_BUF pointer to know
   whether the buffer was mmaped */
#define MADVISE_IBUF(ibuf, addr, len, behav) do { \
    if (ibuf->is_mmaped && madvise(addr, len, behav) == -1) \
	fprintf(stderr, "warning: madvise(0x%p, 0x%08X, %d) failed\n", addr, len, behav); \
} while(0)
#define MADVISE(addr, len, behav) do { \
    if (madvise(addr, len, behav) == -1) \
	fprintf(stderr, "warning: madvise(0x%p, 0x%08X, %d) failed\n", addr, len, behav); \
} while(0)
#else
/* do not use madvise */
#define MADVISE_IBUF(ibuf, addr, len, behav)
#define MADVISE(addr, len, behav)
#endif /* USE_MADVISE */

typedef struct {
    int fd;
    unsigned char *buf;
    size_t size;
    size_t mmap_size; /* `mmap_size' can be greater than `size' */
    int is_mmaped;
} INPUT_BUF;

void load_buf(const char *, INPUT_BUF *);
void unload_buf(INPUT_BUF *);

#endif /* __INPUT_H__ */

/* vi: set ts=8 sw=4: */

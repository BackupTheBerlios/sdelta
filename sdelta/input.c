/* $Id: input.c,v 1.4 2005/01/10 00:16:22 svinn Exp $ */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "input.h"


void load_buf(const char *fname, INPUT_BUF *b) {
    struct stat st;
    size_t alloc_size = 0, cnt;
#if defined(USE_MADVISE) && defined(USE_MAP_ANON_STDIN)
    size_t offset;	/* to cut off the excess */
#endif

    if (fname) {
	if (stat(fname, &st) == -1) {
	    perror(fname);
	    exit(EXIT_FAILURE);
	}
	/* on FreeBSD, open(dirname, O_RDONLY) succeeds */
	if (S_ISDIR(st.st_mode)) {
	    fprintf(stderr, "%s: is a directory\n", fname);
	    exit(EXIT_FAILURE);
	}

	if ((b->fd = open(fname, O_RDONLY)) == -1) {
	    perror(fname);
	    exit(EXIT_FAILURE);
	}

	/* regular file or a symlink, so it is safe to use st.st_size */
	if (S_ISREG(st.st_mode)) {
	    alloc_size = st.st_size;
#ifdef USE_MMAP
	    /* let's try to mmap it directly */
	    if ((b->buf = mmap(NULL, b->mmap_size = b->size = alloc_size, PROT_READ, MAP_PRIVATE, b->fd, 0)) == MAP_FAILED)
		/* mmap failed: try to use malloc */
		fprintf(stderr, "warning: %s: mmap(size = %u, fd = %d) failed\n", fname, alloc_size, b->fd);
	    else {
		/* mmaped successfully, so return */
		b->is_mmaped = 1;
		MADVISE(b->buf, alloc_size, MADV_SEQUENTIAL);
		return;
	    }
#endif
	}
	/* if it is a device, or USE_MMAP is not defined, or mmap failed,
	   use malloc then */
    }
    else {
	/* fname is NULL, so it must be stdin */
	b->fd = 0;
#ifdef USE_MAP_ANON_STDIN
	/* try to prepare MAP_ANON, if available */
	if ((b->buf = mmap(NULL, alloc_size = b->mmap_size = MAX_MAP_ANON, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0)) == MAP_FAILED)
	    /* mmap failed, let's try to use malloc */
	    fprintf(stderr, "warning: stdin: mmap(size = %u, MAP_ANON) failed\n", MAX_MAP_ANON);
	else
	    b->is_mmaped = 1;
#endif
    }

    /* from this point, no mmaps can happen, and the only mmaped
       memory we can encounter is MAP_ANON (that is, stdin) */

    /* use malloc */
#ifdef USE_MAP_ANON_STDIN
    if (! b->is_mmaped) {
#endif
	/* if we know the exact size, add 1 byte */
	alloc_size = alloc_size ? alloc_size + 1 : IBUF_ALLOC_QUANT;
	if (!(b->buf = malloc(alloc_size))) {
	    fprintf(stderr, "%s: malloc(%u) failed\n", fname ? fname : "stdin", alloc_size);
	    exit(EXIT_FAILURE);
	}
#ifdef USE_MAP_ANON_STDIN
    }
#endif

    while (cnt = read(b->fd, b->buf + b->size, alloc_size - b->size)) {
	if (cnt == -1) {
	    perror("read");
	    exit(EXIT_FAILURE);
	}

	b->size += cnt;

	/* input buffer is full: fatal error, if it is a MAP_ANON */
	if (alloc_size == b->size) {
#ifdef USE_MAP_ANON_STDIN
	    if (b->is_mmaped) {
		fprintf(stderr, "stdin: exceeded MAP_ANON size (%u)\n", alloc_size);
		exit(EXIT_FAILURE);
	    }
	    else
#endif
	    /* try to realloc */
	    if (!(b->buf = realloc(b->buf, alloc_size += IBUF_ALLOC_QUANT))) {
		fprintf(stderr, "load_buf: realloc(buf, %u) failed\n", alloc_size);
		exit(EXIT_FAILURE);
	    }
	}
    }

#ifdef USE_MAP_ANON_STDIN
    /* not mmaped, ergo, malloc'ed */
    if (! b->is_mmaped) {
#endif
	/* try to realloc it if needed */
	if (alloc_size - b->size > 1 && !(b->buf = realloc(b->buf, b->size))) {
	    fprintf(stderr, "realloc(buf, %u) failed\n", b->size);
	    exit(EXIT_FAILURE);
	}
#ifdef USE_MAP_ANON_STDIN
    }
#endif

#if defined(USE_MAP_ANON_STDIN) && defined(USE_MREMAP)
    /* if mremap is available, use it */
    if (b->is_mmaped &&
	    (b->buf = mremap(b->buf, MAX_MAP_ANON, b->mmap_size = b->size, MREMAP_MAYMOVE)) == MAP_FAILED) {
	fputs("mremap failed\n", stderr);
	exit(EXIT_FAILURE);
    }
#endif

#if defined(USE_MAP_ANON_STDIN) && defined(USE_MADVISE)
    if (b->is_mmaped) {
#ifndef USE_MREMAP
	/* if mremap is unavailable, use MADVISE to cut off the excess */
	if ((offset = b->size) % PAGE_SIZE)
	    offset += PAGE_SIZE - offset % PAGE_SIZE;
	if (alloc_size > offset)
	    MADVISE(b->buf + offset, alloc_size - offset, MADV_DONTNEED);
#endif
	/* one final madvise */
	MADVISE(b->buf, b->size, MADV_SEQUENTIAL);
    }
#endif

    close(b->fd);
    b->fd = -1; /* unload_buf will know that it is closed */
}


void unload_buf(INPUT_BUF *b) {
    if (! b->buf)
	return;

#if defined(USE_MMAP) || defined(USE_MAP_ANON_STDIN)
    if (b->is_mmaped) {
	if (b->mmap_size && munmap(b->buf, b->mmap_size) == -1)
	    fprintf(stderr, "warning: munmap(0x%p, 0x%08X) failed\n", b->buf, b->mmap_size);
	b->buf = NULL;
	/* close if it is not closed */
	if (b->fd != -1) {
	    close(b->fd);
	    b->fd = -1;
	}
	return;
    }
#endif
    /* regular malloc, so just free() it */
    free(b->buf);
    b->buf = NULL;
}

/* vi: set ts=8 sw=4: */

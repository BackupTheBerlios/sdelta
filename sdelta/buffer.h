#include <stdio.h>
#include <stdlib.h>

unsigned char	*buffer_file		(int   , size_t *);
unsigned char	*buffer_stream		(FILE *, size_t *);
unsigned char	*buffer_known_stream	(FILE *, size_t );

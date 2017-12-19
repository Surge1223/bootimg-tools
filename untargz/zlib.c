

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "zlib.h"

#include <stdio.h>
#include <sys/types.h>


static void _gzerror(gzFile fp, int *errnum, const char **errmsg)
{
	*errmsg = gzerror(fp, errnum);
	if (*errnum == Z_ERRNO) {
		*errmsg = strerror(*errnum);
	}
}

char *zlib_decompress_file(const char *filename, off_t *r_size)
{
	gzFile fp;
	int errnum;
	const char *msg;
	char *buf;
	off_t size = 0, allocated;
	ssize_t result;

	printf("Try gzip decompression.\n");

	*r_size = 0;
	if (!filename) {
		return NULL;
	}
	fp = gzopen(filename, "rb");
	if (fp == 0) {
		_gzerror(fp, &errnum, &msg);
		printf("Cannot open `%s': %s\n", filename, msg);
		return NULL;
	}
	if (gzdirect(fp)) {
		/* It's not in gzip format */
		return NULL;
	}
	allocated = 65536;
	buf = xmalloc(allocated);
	do {
		if (size == allocated) {
			allocated <<= 1;
			buf = xrealloc(buf, allocated);
		}
		result = gzread(fp, buf + size, allocated - size);
		if (result < 0) {
			if ((errno == EINTR) || (errno == EAGAIN))
				continue;
			_gzerror(fp, &errnum, &msg);
			printf("Read on %s of %ld bytes failed: %s\n",
					filename, (allocated - size) + 0UL, msg);
			size = 0;
			goto fail;
		}
		size += result;
	} while(result > 0);

fail:
	result = gzclose(fp);
	if (result != Z_OK) {
		_gzerror(fp, &errnum, &msg);
		printf(" Close of %s failed: %s\n", filename, msg);
	}

	if (size > 0) {
		*r_size = size;
	} else {
		free(buf);
		buf = NULL;
	}
	return buf;
}

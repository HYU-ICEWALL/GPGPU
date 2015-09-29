#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
	#pragma warning (disable : 4996)
#endif

char * file_read(const char *, size_t *);
char * line_read(void);
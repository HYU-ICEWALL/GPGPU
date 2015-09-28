#pragma once
#include <stdio.h>

#ifdef _WIN32 || _WIN64
	#pragma warning (disable : 4996)
#endif

char * file_read(const char *, size_t *);
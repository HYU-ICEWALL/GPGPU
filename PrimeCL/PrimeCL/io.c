#include "io.h"

char * file_read(const char * path, size_t * size)
{
	FILE * file = fopen(path, "rb");
	char * file_text;

	fseek(file, 0, SEEK_END);
	(*size) = ftell(file);
	rewind(file);

	file_text = (char*)malloc(sizeof(char) * (*size) + sizeof(char));
	(*size) = fread(file_text, sizeof(char), (*size), file);
	fclose(file);

	return file_text;
}
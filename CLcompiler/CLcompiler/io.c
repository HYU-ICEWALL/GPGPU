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
char * line_read()
{
	size_t size = 0;
	char ch, * result = (char*)malloc(sizeof(char));

	while ((ch = getchar()) != EOF && ch != '\n')
	{
		result = (char*)realloc(result, sizeof(char) * size++ + sizeof(char));
		result[size - 1] = ch;
		result[size] = 0;
	}
	return result;
}
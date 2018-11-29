#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

int iswhitespace(char c);
char* stristr(const char* haystack, const char* needle);
void trim(char* str);
void* recalloc(void* array, size_t elem_size, int old_count, int new_count);
void* realloc_zero(void* pBuffer, size_t oldSize, size_t newSize);

#endif

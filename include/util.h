#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <time.h>

int iswhitespace(char c);
char* stristr(const char* haystack, const char* needle);
void trim(char* str);
void* recalloc(void* array, size_t elem_size, int old_count, int new_count);
void* remove_null_elements(void* array, size_t elem_size, int* count);
double measure_duration(double bpm, double metre);
const char* get_extension(const char *file);
struct timespec timespec_diff(struct timespec start, struct timespec end);
struct timespec timespec_add_ns(struct timespec time, long ns);

#endif

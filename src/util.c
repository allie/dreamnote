#include "util.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

int iswhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

char* stristr(const char* haystack, const char* needle) {
	do {
		const char* h = haystack;
		const char* n = needle;

		while (tolower((unsigned char)*h) == tolower((unsigned char)*n) && *n) {
			h++;
			n++;
		}

		if (*n == 0) {
			return (char*)haystack;
		}
	} while (*haystack++);

	return 0;
}

void trim(char* str) {
	int dest;
	int src = 0;
	int len = strlen(str);

	// Advance the starting pointer until there is no more whitespace
	while (iswhitespace(str[src])) {
		src++;
	}

	// Set trailing whitespace to NULL
	for (dest = len - 1; iswhitespace(str[dest]); dest--) {
		str[dest] = '\0';
	}

	// Shift the string back to the starting point so it can be freed later
	memmove(str, str + src, strlen(str + src) + 1);
}

void* recalloc(void* array, size_t elem_size, int old_count, int new_count) {
	void* new_array = realloc(array, elem_size * new_count);

	if (new_count > old_count && new_array) {
		memset((char*)new_array + (old_count * elem_size), 0, (new_count - old_count) * elem_size);
	}

	return new_array;
}

void* remove_null_elements(void* array, size_t elem_size, int* count) {
	int actual_count = 0;

	// Get an actual count of non-null elements
	for (int i = 0; i < *count; i++) {
		if (array + i * elem_size != NULL) {
			actual_count++;
		}
	}

	void* new_array = calloc(elem_size, actual_count);
	int offset = 0;

	for (int i = 0; i < *count; i++) {
		if (array + i * elem_size != NULL) {
			memcpy(new_array + (offset * elem_size), array, elem_size);
			offset++;
		}
	}

	free(array);
	*count = actual_count;
	return new_array;
}

// Returns the number of seconds in a measure at the given BPM and metre
double measure_duration(double bpm, double metre) {
	return 60.0 / bpm * 4 * metre;
}

const char* get_extension(const char *file) {
	const char *dot = strrchr(file, '.');
	if (!dot || dot == file) {
		return "";
	}
	return dot + 1;
}

struct timespec timespec_diff(struct timespec start, struct timespec end) {
	struct timespec temp;

	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1E9 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}

	return temp;
}

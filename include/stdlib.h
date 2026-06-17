#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>

void k_mem_init(size_t max);

int atoi(const char *nptr);
int abs(int j);

void *malloc(size_t size);
void free(void *ptr);

void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

#endif // _STDLIB_H


#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

// Assembly functions
extern void *memset(void *ptr, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);

extern char *strcpy(char *dest, const char *src);
extern char *strcat(char *dest, const char *src);

// C functions
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

int strsplit(char *string, char splitter, size_t n, char **split);
char *strdup(const char *src);

#endif // _STRING_H

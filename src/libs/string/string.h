#ifndef STRING_H
#define STRING_H

#include "stdint.h"
#include "stdlib.h"

// String compare
uint8_t strcmp(char *s1, char *s2);
uint8_t strncmp(char *s1, char *s2, uint32_t n);

void *memcpy(void *dest, const void *src, uint32_t n);
void *memset(void *s, int c, uint32_t n);

// Split with spaces/tabs/newlines
void copy_first(char *s, char *buff);
char *get_second(char *s);

uint32_t atoi(char *s);
uint32_t atoh(char *s);

#endif // STRING_H


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
uint32_t firstlen(char *s);
char *parse_second(char *s);

uint32_t strlen(const char *s);

// Replace
char *replace(const char *s, const char *from, const char *to, uint32_t maxlen);
char *replacen(
    const char *s,
    const char *from, const char *to,
    uint32_t maxlen, uint32_t n);

uint32_t atoi(char *s);
uint32_t atoh(char *s);

char *itoa(uint32_t num, char *str, uint32_t base);

#endif // STRING_H


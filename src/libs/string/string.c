#include "string.h"

uint8_t strcmp(char *s1, char *s2) {
  if (s1 == NULL || s2 == NULL) return 0;

  while (*s1 == *s2) {
    if (*s1 == 0) return 1;
    ++s1;
    ++s2;
  }
  return 0;
}

uint8_t strncmp(char *s1, char *s2, uint32_t n) {
  if (s1 == NULL || s2 == NULL) return 0;
  if (n == 0) return 1;

  while (*s1 == *s2) {
    if (--n == 0 || *s1 == 0) return 1;
    ++s1;
    ++s2;
  }
  return 0;
}

void *memcpy(void *dest, const void *src, uint32_t n) {
  char *dest_c  = (char*)dest;
  const char *src_c = (const char*)src;
  while (n > 0) {
    *dest_c = (char)*src_c;
    ++dest_c;
    ++src_c;
    --n;
  }
  return dest;
}

void *memset(void *s, int c, uint32_t n) {
  char *s_c = (char*)s;
  char c_c = (char)c;
  while (n > 0) {
    *s_c = c_c;
    ++s_c;
    --n;
  }
  return s;
}

void copy_first(char *s, char *buff) {
  if (buff == NULL || s == NULL) return;

  while (*s != 0 && *s != ' ') {
    *buff = *s;
    ++s;
    ++buff;
  }

  *buff = 0;
}

char *get_second(char *s) {
  if (s == NULL) return NULL;

  // Skip first
  while (*s != 0 && *s != ' ')
    ++s;

  // Skip spacing
  while (*s == ' ')
    ++s;

  if (*s == 0) return NULL;
  return s;
}

uint32_t atoi(char *s) {
  uint32_t num = 0;
  while (*s != 0) {
    num *= 10;
    num += *s - '0';
    ++s;
  }
  return num;
}

uint32_t atoh(char *s) {
  uint32_t num = 0;

  while (*s != 0) {
    num <<= 4;
    if (*s > '0' && *s <= '9')
      num += (*s - '0');
    else if (*s >= 'A' && *s <= 'F')
      num += (*s - 'A') + 10;
    ++s;
  }

  return num;
}

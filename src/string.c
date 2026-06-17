#include <string.h>


int memcmp(const void *s1, const void *s2, size_t n) {
  uint8_t *c1 = (uint8_t*)s1;
  uint8_t *c2 = (uint8_t*)s2;
  while (n > 0) {
    if (*c1 != *c2) {
      return (int)*c1 - (int)*c2;
    }

    --n;
    ++c1;
    ++c2;
  }
  return 0;
}

size_t strlen(const char *s) {
  if (s == NULL) return 0;

  size_t c = 0;
  while (*s != 0) {
    ++c;
    ++s;
  }
  return c;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 == *s2) {
    if (*s1 == 0) return 0; // Identical
    ++s1;
    ++s2;
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) return 0;
  while (n > 1 && *s1 == *s2) {
    if (*s1 == 0) return 0;
    ++s1;
    ++s2;
    --n;
  }
  return *s1 - *s2;
}

char *strcpy(char *dest, const char *src) {
  if (dest == NULL || src == NULL) return NULL;

  char *dest_c = dest;
  while (*src != 0) {
    *dest = (char)*src;
    ++dest;
    ++src;
  }
  *dest = 0;
  return dest_c;
}

char *strncpy(char *dest, const char *src, size_t n) {
  if (dest == NULL || src == NULL) return NULL;

  char *dest_c = dest;
  while (*src != 0 && n > 0) {
    *dest = (char)*src;
    ++dest;
    ++src;
    --n;
  }
  while (n > 0) {
    *dest = 0;
    ++dest;
    --n;
  }
  return dest_c;
}


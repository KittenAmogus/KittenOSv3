#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *c1 = (const uint8_t *)s1;
  const uint8_t *c2 = (const uint8_t *)s2;

  while (n > 0) {
    if (*c1 != *c2)
      return (int)*c1 - (int)*c2;
    ++c1;
    ++c2;
    --n;
  }

  return 0;
}

size_t strlen(const char *s) {
  size_t n = 0;
  while (*s != 0) {
    ++s;
    ++n;
  }
  return n;
}

int strcmp(const char *s1, const char *s2) {
  const uint8_t *c1 = (const uint8_t *)s1;
  const uint8_t *c2 = (const uint8_t *)s2;

  while (*c1 == *c2) {
    if (*c1 == 0)
      return 0;
    ++c1;
    ++c2;
  }

  return (int)*c1 - (int)*c2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const uint8_t *c1 = (const uint8_t *)s1;
  const uint8_t *c2 = (const uint8_t *)s2;

  while (n > 0 && *c1 == *c2) {
    if (*c1 == 0)
      return 0;

    ++c1;
    ++c2;
    --n;
  }

  if (*c1 == 0 || *c2 == 0)
    return 0;

  if (*c1 != *c2) {
    return (int)*c1 - (int)*c2;
  }

  return 0;
}

char *strchr(const char *s, int c) {
  char ch = (char)c;
  while (*s != ch && *s != 0)
    ++s;
  if (ch == *s)
    return (char *)s;
  return NULL;
}

char *strstr(const char *haystack, const char *needle) {
  int needle_len = strlen(needle);
  if (needle_len < 1)
    return (char *)haystack;

  while (*haystack != 0) {
    if (strncmp(haystack, needle, needle_len) == 0) {
      return (char *)haystack;
    }
    ++haystack;
  }
  return NULL;
}

char *strdup(const char *src) {
  if (src == NULL)
    return NULL;

  int len = strlen(src);
  char *cpy = malloc(len + 1);
  if (cpy == NULL)
    return NULL;

  return strcpy(cpy, src);
}

int strsplit(char *string, char splitter, size_t n, char **split) {
  if (string == NULL || split == NULL)
    return -1;

  if (splitter == 0 || n == 0 || *string == 0) {
    split[0] = string;
    return 1;
  }

  size_t count = 0;
  do {
    split[count] = string;

    while (*string != 0 && *string != splitter)
      ++string;

    while (*string != 0 && *string == splitter) {
      *string = 0;
      ++string;
    }

    if (*string == 0)
      break;

    ++count;
  } while (count < n);

  if (n == count)
    split[count] = string;

  return count + 1;
}

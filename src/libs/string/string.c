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


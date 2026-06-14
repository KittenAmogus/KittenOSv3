#include "string.h"

const char *nums_letters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

uint8_t strcmp(char *s1, char *s2) {
  if (s1 == NULL || s2 == NULL) return 1;

  while (*s1 == *s2) {
    if (*s1 == 0) return 0;
    ++s1;
    ++s2;
  }
  return 1;
}

uint8_t strncmp(char *s1, char *s2, uint32_t n) {
  if (s1 == NULL || s2 == NULL) return 1;
  if (n == 0) return 0;

  while (*s1 == *s2) {
    if (--n == 0 || *s1 == 0) return 0;
    ++s1;
    ++s2;
  }
  return 1;
}

void *memcpy(void *dest, const void *src, uint32_t n) {
  uint8_t *dest_c  = (uint8_t*)dest;
  const uint8_t *src_c = (const uint8_t*)src;
  while (n > 0) {
    *dest_c = *src_c;
    ++dest_c;
    ++src_c;
    --n;
  }
  return dest;
}

void *memset(void *s, int c, uint32_t n) {
  uint8_t *s_c = (uint8_t*)s;
  uint8_t c_c = (uint8_t)c;
  while (n > 0) {
    *s_c = c_c;
    ++s_c;
    --n;
  }
  return s;
}

uint32_t firstlen(char *s) {
  uint32_t len = 0;
  while (*s != 0 && *s != ' ') {
    ++s;
    ++len;
  }
  return len;
}

char *parse_second(char *s) {
  while (*s != 0 && *s != ' ')
    ++s;

  while (*s == ' ')
    ++s;

  if (*s == 0) return NULL;
  return s;
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

uint32_t strlen(const char *s) {
  if (s == NULL) return 0;
  uint32_t len = 0;
  while (*s != 0) {
    ++len;
    ++s;
  }
  return len;
}

char *replace(
  const char *s,
  const char *from, const char *to,
  uint32_t maxlen) {
  // Allocate buffer
  void *buffer = malloc(maxlen);
  if (buffer == NULL) return NULL;

  // Pointers
  char *dest = (char*)buffer;
  char *src  = (char*)s;

  // Lengths
  uint32_t fromlen = strlen(from);
  uint32_t tolen   = strlen(to);
  
  uint32_t freespc = maxlen - 1;
  while (*src != 0 && freespc > 0) {
    if (strncmp(src, from, fromlen) == 0) {
      if (freespc < tolen) break; // Not enough space
      // Copy to buffer
      memcpy(dest, to, tolen);
      dest += tolen;
      src  += fromlen;
      freespc -= tolen;
    } else {
      // Copy char
      *dest = *src;
      ++dest;
      ++src;
      --freespc;
    }
  }

  *dest = 0; // Null-terminated string
  return (char*)buffer;
}


char *replacen(
  const char *s,
  const char *from, const char *to,
  uint32_t maxlen, uint32_t n) {
  // Allocate buffer
  void *buffer = malloc(maxlen);
  if (buffer == NULL) return NULL;

  // Pointers
  char *dest = (char*)buffer;
  char *src  = (char*)s;

  // Lengths
  uint32_t fromlen = strlen(from);
  uint32_t tolen   = strlen(to);

  uint32_t nc = n;
  uint32_t freespc = maxlen - 1;
  while (*src != 0 && freespc > 0) {
    if (strncmp(src, from, fromlen) == 0) {
      if (freespc < tolen || nc < 1) break; // Not enough space
      // Copy to buffer
      memcpy(dest, to, tolen);
      dest += tolen;
      src  += fromlen;
      --nc;
      freespc -= tolen;
    } else {
      // Copy char
      *dest = *src;
      ++dest;
      ++src;
      --freespc;
    }
  }

  *dest = 0; // Null-terminated string
  return (char*)buffer;
}


char *itoa(uint32_t num, char *str, uint32_t base) {
  char *strc = str + 11;  // last
  uint32_t nc = num;      // Copy of N
  uint32_t rem = 0;

  *strc = 0;

  if (num == 0) {
    --strc;
    *strc = '0';
    return strc;
  }

  while (nc > 0) {
    rem = nc % base;
    nc /= base;
    --strc;
    *strc = nums_letters[rem];
  }
  return strc;
}


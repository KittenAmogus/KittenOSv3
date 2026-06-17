#include <stdlib.h>


int atoi(const char *nptr) {
  if (nptr == NULL) return 0;
  if (*nptr == 0) return 0;

  int n = 0;
  int sign = 1;

  // Negative
  if (*nptr == '-') {
    sign = -1;
    ++nptr;
  }

  while (*nptr >= '0' && *nptr <= '9') {
    n *= 10;
    n += *nptr - '0';
    ++nptr;
  }

  return n * sign;
}

int abs(int j) {
  return (j < 0 ? -j : j);
}


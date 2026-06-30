#include <stdlib.h>

uint32_t atoi(const char *s);

int32_t strtol(const char *nptr, char **endptr, int base);
uint32_t strtoul(const char *nptr, char **endptr, int base);

void exit(uint32_t code);
void abort(void);

uint32_t abs(int32_t i);
uint64_t labs(int64_t l);
div_t div(int numer, int denom);

uint32_t rand(void);
void srand(uint32_t seed);

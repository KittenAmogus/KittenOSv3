#include "random.h"

static uint32_t _prev_random = 0;

// Set random seed
void srand(uint32_t seed) {
  _prev_random = seed;
}

// POSIX Standart random
unsigned int rand(void) {
  _prev_random = (_prev_random * 1103515245) + 12345;
  return (unsigned int)(_prev_random / 0x10000) & 0x7FFF;
}


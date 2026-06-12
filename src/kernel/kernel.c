#include "stdio.h"

// Entry
int main(void) {
  vgaAttrs(2, 8);
  char *str = "Hello, World!\nNewline!\nREW\rZ";
  puts(str);

  return 0;
}


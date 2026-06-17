#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <vga.h>


int kmain(multiboot_data_t *mbi) {
  k_mem_init(mbi->mem_upper);
  vga_init(1);
  k_stdio_init();
  vga_clear();

  puts("Hello, user!");
  puts("This message has been");
  puts("Writed to VGA buffer via");
  puts("FD 1 (stdout)");

  return 0;
}


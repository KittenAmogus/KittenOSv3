#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <vga.h>
#include <gdt.h>
#include <idt.h>

#include <keyboard.h>

extern void keyboard_handler(void);


static const char * const _HELLO_MSG = (
  "Hello, user of KittenOS!\n");


static void _init_kernel(multiboot_data_t *mbi) {
  gdt_init();
  idt_init();

  uint32_t max_ram_bytes = 1024 * 1024 + (mbi->mem_upper * 1024);
  k_mem_init(max_ram_bytes);
  k_stdio_init();
}


int kmain(multiboot_data_t *mbi) {
  _init_kernel(mbi);

  if (malloc(128) == NULL) {
    return 2;
  }

  /* Creating VGA stdout */
  stdout = vga_init(STDOUT);

  vga_clear();
  puts(_HELLO_MSG);

  /* Creating keyboard stdin */
  stdin = kb_init(STDIN);
  if (stdin->buffer == NULL) {
    stdin->buffer = malloc(1024);
  }

  if (malloc(128) == NULL) {
    puts("Random malloc is null");
  }

  if (stdin == NULL) {
    puts("STDIN is null");
    return 1;
  } else if (stdin->buffer == NULL) {
    puts("STDIN buffer is null");
    return 2;
  }

  int ch;

  puts("Start typing...");
  while (1) {
    ch = getchar();
    if (ch != -1) {
      if (ch == '\b') {
        putchar('<');
      }
      else
        putchar((char)ch);
    } else {
      putchar((int)'X');
    }
  }

  puts("\n[INFO] Kernel is now in idle mode\n");
  while (1) {
    __asm__ volatile ("hlt");
  }

  return 0;
}


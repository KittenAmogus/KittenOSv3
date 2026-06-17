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

int kmain(multiboot_data_t *mbi) {
  gdt_init();
  idt_init();

  k_mem_init(mbi->mem_upper); // Heap

  // Vga IO
  vga_init(1);
  vga_clear();

  // Stdin
  /*stdin = malloc(sizeof(FILE));
  _file_descriptors[0] = stdin;
  if (_file_descriptors[0] != stdin) puts("WTF");

  stdin->fd = 0;
  stdin->flags = 0;
  stdin->buffer = malloc(128);
  if (stdin->buffer == NULL) puts("0Even init is null");
  stdin->buffer_size = 128;
  stdin->buffer_pos = 0;*/

  // Init stdio (stdout = vga)
  k_stdio_init();
  if (_file_descriptors[0] != stdin) puts("WTF2");
  if (stdin == NULL) puts("1Even init is null");
  kb_init();
  if (stdin == NULL) puts("2Even init is null");

  puts("Hello, user!");
  puts("This message has been");
  puts("Writed to VGA buffer via");
  puts("FD 1 (stdout)");

  char *line = NULL;
  size_t n = 0;

  if (stdin == NULL) {
    puts("NULL Stdin");
  }
  if (stdin->buffer == NULL) {
    puts("NULL Buffer");
  }

  vga_clear(); // Очищаем старый мусор VGA
  puts("FUCK YES!");
  while (1) {
    // puts("=> ");
    getline(&line, &n, stdin);

    // if (getline(&line, &n, stdin) < 0) break;
    if (line != NULL) {
      puts(line);
    }
  }

  puts("\n!! Infinity halt mode now !!\n");
  while (1) {
    __asm__ volatile ("hlt");
  }

  return 0;
}


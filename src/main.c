#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <drivers/gdt.h>
#include <drivers/idt.h>
#include <drivers/pic.h>
#include <drivers/vga.h>

#include <builtin/apps.h>
#include <builtin/shell.h>

extern FILE *_file_descriptors[];

static int _init_stdin(void) {
  stdin = malloc(sizeof(FILE));

  stdin->fd = 0;
  stdin->buffer = malloc(2048);
  stdin->buffer_size = (stdin->buffer == NULL ? 0 : 2048);
  stdin->buffer_pos = 0;
  _file_descriptors[0] = stdin;

  if (stdin->buffer_size == 0) {
    puts("[ERR] stdin_kbd->buffer_pos == 0");
    return ENOMEM;
  }

  return SUCCESS;
}

static inline void _kernel_init(multiboot_data_t *mbi) {
  gdt_init();
  idt_init();
  pic_remap();

  k_mem_init((mbi->mem_upper << 10) + (1 << 20));
  k_stdio_init();
}

int kmain(uint32_t magic, uint32_t mboot_addr) {
  /* Check boot info */
  if (magic != 0x2BADB002)
    return EINVAL;
  if (mboot_addr == 0)
    return EINVAL;

  /* Init kernel */
  int err;
  _kernel_init((multiboot_data_t *)mboot_addr);

  err = _init_stdin();
  if (err != SUCCESS)
    return err;

  /* Prepare VGA */
  vga_clear();
  puts("Hello, user!");
  puts(" * KittenOSv4");

  /* Run shell */
  err = shell_app_func(0, NULL);
  if (err != SUCCESS)
    return err;

  /* Exit code */
  return SUCCESS;
}

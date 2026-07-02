#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <drivers/gdt.h>
#include <drivers/idt.h>
#include <drivers/pic.h>
#include <drivers/vga.h>

#include <drivers/disk.h>
#include <drivers/fs/fat.h>
#include <drivers/ramdisk.h>

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

  /* Reg drivers */
  vfs_register_driver(&fat_driver);

  /* Prepare VGA */
  vga_clear();
  puts("Hello, user!");
  puts(" * KittenOSv4");

  /* Run shell */
  const char *path;
  path = (const char *)rd_createdev(128 << 10);
  printf("MAKEFS: %x\n", vfs_makefs(path));
  printf("MOUNTFS: %x\n", vfs_mountfs(path, "/mnt"));
  printf("UMOUNTFS: %x\n", vfs_umountfs(0, "/mnt"));
  printf("UMOUNTFS: %x\n", vfs_umountfs(0, "/mnt"));
  printf("MOUNTFS: %x\n", vfs_mountfs(path, "/mnt"));
  printf("MOUNTFS: %x\n", vfs_mountfs(path, "/mnt"));
  printf("UMOUNTFS: %x\n", vfs_umountfs(0, "/mnt"));

  err = shell_app_func(0, NULL);
  if (err != SUCCESS)
    return err;
  /* Exit code */
  return SUCCESS;
}

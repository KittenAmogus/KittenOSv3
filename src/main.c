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

#define ASSERT(exp, err_code, lbl)                                             \
  do {                                                                         \
    if (!(exp)) {                                                              \
      err = (err_code);                                                        \
      err_msg = #exp;                                                          \
      goto lbl;                                                                \
    }                                                                          \
  } while (0)

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

  /* ===============================
   *        MAIN KERNEL CODE
   * =============================== */

  shell_app_func(0, NULL);

  char *err_msg;
  err = 0;

  vga_clear();
  puts("Running FAT32 tests...\n");

  puts("Creating ramdisk");
  blk_dev_t *ramdisk = rd_createdev(64 << 20);
  ASSERT(ramdisk != NULL, ENOMEM, kernel_panic);

  puts("Registering ramdisk");
  ASSERT(vfs_register_device(ramdisk, "ram0") == 0, ENODEV, kernel_panic);

  puts("Making filesystem");
  err = vfs_makefs("/dev/ram0", "fat");
  ASSERT(err == 0, err, kernel_panic);

  /*puts("Mounting filesystem");
  err = vfs_mountfs("/dev/ram0", "/", "fat");
  ASSERT(err == 0, err, kernel_panic);*/

  char *b = malloc(512 * 20);
  ramdisk->read_sector(ramdisk, 480, b, 20);

  /*
  asm volatile("hlt");
  asm volatile("hlt");
  asm volatile("hlt");
  */
  int j = 0;
  int a = 0;
  size_t wr = 0;
  extern int _vga_cursor_y;
  do {
    printf("%d | ", j);
    a = 0;
    for (int i = 0; i < 32; ++i) {
      if (*b != 0)
        a = 1;
      printf("%b", *b);
      ++b;
      ++wr;
    }
    ++j;
    printf(" |\n");

    if (a == 0)
      _vga_cursor_y--;

    if (wr > 20480)
      break;
    // asm volatile("hlt");
  } while (1);

  puts("Opening directory /root/test");
  void *handle = vfs_opendir("/root/test");
  ASSERT(handle != NULL, ENOENT, kernel_panic);

  puts("Opening directory /");
  handle = vfs_opendir("/");
  ASSERT(handle != NULL, ENOENT, kernel_panic);

  puts("Creating /FILE*.TXT");
  err = vfs_creat("/FILE1.TXT");
  err = vfs_creat("/FILE2.TXT");
  err = vfs_creat("/FILE3.TXT");
  ASSERT(err == 0, err, kernel_panic);

  puts("Allocating dirent_t *dirent");
  dirent_t *dirent = malloc(sizeof(dirent_t));
  ASSERT(dirent != NULL, ENOMEM, kernel_panic);

  puts("Reading directory /:");
  puts("/");

  int errcode = 0;
  while (errcode > -1) {
    errcode = vfs_readdir(handle, dirent);
    ASSERT(errcode > -2, EIO, kernel_panic);
    printf("ERRCODE %d\n", errcode);

    if (errcode != EOF) {
      printf("|- [%s] (%d) '%s'\n",
             (dirent->type == DIRENT_UNKNOWN
                  ? "----"
                  : (dirent->type == DIRENT_FILE ? "FILE" : "DIR ")),
             dirent->size, dirent->name);
    }
  }
  puts("Reading done");

  puts("Unmounting /");
  ASSERT(vfs_umountfs("/dev/ram0", "/") == 0, EINVAL, kernel_panic);

  puts("Freeing");
  free(handle);
  free(dirent);

  puts("[KERNEL EXIT]");
  return SUCCESS;

kernel_panic:
  printf("\n\r[KERNEL PANIC] FAILED EXPRESSION: (%s) (code: %d)\n\r", err_msg,
         err);
  return err;
}

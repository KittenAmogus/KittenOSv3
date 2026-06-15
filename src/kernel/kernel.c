#include "kernel.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "random.h"

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "pic/pic.h"

#include "apps/apps.h"

#include "drivers/vga/vga.h"
#include "kernel/fs/fs.h"

uint32_t _grub_ram_size;

static void init_system(mboot_info *mbi) {
  gdt_init();
  idt_init();
  pic_remap();

  asm volatile ("sti");

  size_t ram_size = (
    mbi->flags & 0x01 ? mbi->mem_upper + 1024 : (16 << 10));
  initHeap(ram_size << 10); // KB to MB
  _grub_ram_size = (mbi->mem_upper << 10) + (1 << 20);

  vga_attrs(0x07, 0x01);
  clear_screen();

  puts(_hello_message);
}


extern app_t app_table[];

uint32_t kmain(mboot_info *mbi) {
  uint32_t status;
  init_system(mbi);

  puts("Trying to mount disk...");
  status = fs_mountfs();

  uint8_t tries = 0;
  while (status != 0 && tries < 8) {
    printf("Error code %d, retrying... %d\n", status, tries);
    fs_makefs();
    status = fs_mountfs();
    ++tries;
  }
  puts("Mounted successfully\n");

  app_t *shell_app = &app_table[0];
  shell_app->func(NULL);

  srand(0x12345678);
  printf("Kernel exit\n");
  return 0;
}


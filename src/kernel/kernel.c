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


static void shell(void) {
  char *prompt = PROMPT;

  uint8_t status = 0;
  char *buff = NULL;
  char *cmd = malloc(CMD_MAX);
  char *args = malloc(CMD_MAX);

  while (true) {
    printf("[%d] %s", status, prompt);
    buff = getline(CMD_MAX);
    if (buff == NULL) break;

    if (*buff == 0) continue;

    copy_first(buff, cmd);
    args = get_second(buff);

    uint8_t found = 0;

    for (uint16_t i=0; i<app_count; ++i) {
      // static const shell_cmd cmd_table[] = {
      const app_t *app_ptr = &(app_table[i]);
      if (strcmp(app_ptr->name, cmd)) {
        status = app_ptr->func(args);
        found = 1;
        break;
      }
    }

    // Unhandled cmd
    if (!found) {
      printf("Invalid command: '%s'\n", cmd);
      status = 0xFF;
    }

    free(buff);
  }

  free(cmd);
  free(args);

  printf("Shell exit");
  return;
}


uint32_t kmain(mboot_info *mbi) {
  init_system(mbi);
  srand(0x12345678);

  shell();

  printf("Kernel exit\n");
  return 0;
}


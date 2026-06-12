#include "kernel.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "random.h"

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "pic/pic.h"

#include "drivers/vga/vga.h"

// TODO: Shell.h
typedef uint8_t (*shell_func)(char *args);

typedef struct {
  const char *name;
  shell_func func;
} shell_cmd;


static uint8_t cmd_clear(char *args) {
  clear_screen();
  return 0;
}

static uint8_t cmd_echo(char *args) {
  if (args != NULL) {
    puts(args);
    return 0;
  } else {
    puts("Usage: echo <chars>");
    return 1;
  }
}

static uint8_t cmd_help(char *args) {
  puts("Supported commands: ");
  puts(" * help  - Show this menu");
  puts(" * clear - Clear screen");
  puts(" * echo  - Print text on screen");
  return 0;
}

static const shell_cmd cmd_table[] = {
  {"help", cmd_help},
  {"echo", cmd_echo},
  {"clear", cmd_clear},
};
#define CMD_COUNT (sizeof(cmd_table) / sizeof(shell_cmd))

// TODO: Shell.h


static void init_system(mboot_info *mbi) {
  gdt_init();
  idt_init();
  pic_remap();

  asm volatile ("sti");

  size_t ram_size = (
    mbi->flags & 0x01 ? mbi->mem_upper + 1024 : (16 << 10));
  initHeap(ram_size << 10); // KB to MB

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

    for (uint16_t i=0; i<CMD_COUNT; ++i) {
      // static const shell_cmd cmd_table[] = {
      const shell_cmd *sptr = &(cmd_table[i]);
      if (strcmp(sptr->name, cmd)) {
        status = sptr->func(args);
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


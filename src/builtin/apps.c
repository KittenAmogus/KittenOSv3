#include <builtin/shell.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/io.h>
#include <drivers/vga.h>

#include <builtin/apps.h>

int help_app_func(int argc, char **argv) {
  app_t app;

  puts("Available commands:");

  for (size_t i = 0; i < app_table_size; ++i) {
    app = app_table[i];
    if (app.name != NULL && app.descr != NULL) {
      printf(" * %s - %s\n", app.name, app.descr);
    } else {
      printf(" * -- BROKEN APP --\n");
    }
  }
  printf(" * exit - Exits shell\n");

  return SUCCESS;
}

int reboot_app_func(int argc, char **argv) {
  outb(0x64, 0xFE);
  return SUCCESS;
}

int clear_app_func(int argc, char **argv) {
  vga_clear();
  return SUCCESS;
}

extern int shell_app_func(int argc, char **argv);

const app_t app_table[] = {
    {"help", "Shows available command list", help_app_func},
    {"clear", "Clears VGA screen", clear_app_func},
    {"reboot", "Reboots your computer ( outb(64h, FEh) )", reboot_app_func},
    {"shell", "This is basic shell for KittenOSv3", shell_app_func},
};

const size_t app_table_size = (sizeof(app_table) / sizeof(const app_t));

#include "apps.h"

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "random.h"

static const char *noargs_echo = "Usage: echo <string>";

static uint32_t app_clear(char *args) {
  clear_screen();
  return 0;
}

static uint32_t app_echo(char *args) {
  if (args == NULL) {
    puts(noargs_echo);
    return 1;
  }
  puts(args);
  return 0;
}

static uint32_t app_help(char *args) {
  puts("Available commands: ");
  for (uint32_t i=0; i<app_count; ++i) {
    app_t *app = &(app_table[i]);
    if (app == NULL || app->name == NULL || app->descr == NULL)
      printf(" * [CORRUPTED]\n");
    else
      printf(" * %s - %s\n", app->name, app->descr);
  }

  return 0;
}

static uint32_t app_fetch(char *args) {
  puts(" /\\_/\\  OS name | KittenOSv3");
  puts("( o.o ) OS type | 32-bit Monolithic");
  puts("  >^<   Author  | @kittenamogus\n");
  puts("        <><><><><><><><><><><><><><>");
  printf("        RAM     | %d/%d MB free\n",
    get_heap_free() >> 20, get_heap_size() >> 20);
  return 0;
}

static uint32_t app_malloc(char *args) {
  size_t size;

  if (args[0] == '0' && args[1] == 'x') {
    size = atoh(args + 2);
  } else {
    size = atoi(args);
  }

  if ((int32_t)size <= 0) {
    puts("Cannot allocate <= 0 bytes");
    return 2;
  }

  uint8_t *buff = malloc(size);
  if (buff == NULL) {
    puts("Alloc error, not enough RAM");
    return 1;
  }
  printf(
    "Allocated %d (0x%x) bytes, addr: %d (0x%x)\n",
    size, size, (uint32_t)buff, (uint32_t)buff);
  return 0;
}


static uint32_t app_free(char *args) {
  void *ptr;

  if (args[0] == '0' && args[1] == 'x') {
    ptr = (void*)atoh(args + 2);
  } else {
    ptr = (void*)atoi(args);
  }

  if (ptr == NULL || ptr <= META_SIZE) {
    puts("Invalid pointer, too small");
    return 2;
  }

  Block *b = (Block*)((uint8_t*)ptr - META_SIZE);
  if (b->magic != BLOCK_MAGIC) {
    if (b->magic == ~BLOCK_MAGIC) {
      puts("Double-free attempt!");
      return 3;
    }
    puts("Invalid addr");
    return 1;
  }

  free(ptr);
  puts("Freed block");
  return 0;
}


app_t app_table[] = {
  {"clear",   "Fills screen with empty chars", app_clear},
  {"echo",    "Prints args in stdout", app_echo},
  {"help",    "Prints list of apps with their descriptions", app_help},
  {"fetch",   "Prints some OS and hardware info", app_fetch},
  {"malloc",  "Allocate <args> bytes", app_malloc},
  {"free",    "Free RAM block at <args> addr", app_free},
};
const uint32_t app_count = sizeof(app_table) / sizeof(app_t);


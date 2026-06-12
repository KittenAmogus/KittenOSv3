#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "pic/pic.h"

const char * const _hello_message = (
  "Hello, User!\n"
  "This is operating system, written with C and NASM, using\n"
  "- The little book about OsDev\n"
  "By @kittenamogus\n" "With help from Google AI\n"
);

struct mboot_info {
  unsigned int flags;
  unsigned int mem_lower;
  unsigned int mem_upper;
} __attribute__((packed));

void clearScreen(void) {
  cursorPos(0, 0);
  for (int a=0; a<(80 * 25); ++a) {
    putc(0);
  }
  cursorPos(0, 0);
}

// Returns 1 if s1 == s2
int stringcmp(char *s1, char *s2) {
  if (s1 == NULL || s2 == NULL) return 0;

  while (true) {
    // Not equal if s1[i] != s2[i]
    if (*s1 != *s2) return 0;

    // Not equal if one is shorter
    else if ((*s1 == 0) ^ (*s2 == 0)) return 0;
 
    // Equal
    else if (*s1 == 0) return 1;

    ++s1;
    ++s2;
  }
}

int cmd_cmp(char *s, char *cmd) {
  while (*cmd != 0) {
    if (*s == 0 || *s == ' ') return 0;
    else if (*s != *cmd) return 0;

    ++s;
    ++cmd; }

  return (*s == ' ' || *s == 0);
}

char *get_args(char *s) {

  // Skip first word
  while (*s != ' ' && *s != 0) {
    ++s;
  }

  // Skip spacing
  while (*s == ' ') {
    ++s;
  }

  // Leading spaces protection
  if (*s == 0) return NULL;

  // Arg start
  return s;
}

// Entry
int kmain(struct mboot_info *mbi) {
  // Init interrupts
  gdt_init();
  idt_init();
  pic_remap();
  asm volatile ("sti");

  // Init heap
  size_t ramSize = (mbi->flags & 0x01 ? mbi->mem_upper + 1024 : (16 << 20));
  initHeap(ramSize);

  /* === SETUP COMPLETE === */

  vgaAttrs(7, 1); // Reset VGA
  clearScreen();
  puts(_hello_message);

  /* === MAIN PROCESS === */

  char *prompt = "user $ ";
 
  char *buff;

  while (true) {
    putstr(prompt);
    buff = getline(256);

    if (cmd_cmp(buff, "clear")) {
      clearScreen();
    }

    else if (cmd_cmp(buff, "echo")) {
      char *args = get_args(buff);
      if (args != NULL)
        puts(args);
      else
        puts("Where are your args? Give me more, now!");
    }

    else if (cmd_cmp(buff, "exit")) {
      putstr("Exit\n");
      break;
    }
  }

  puts("Warning! Triple fault will be executed...");
  asm volatile (
    "lidt (%0)\n"
    "div %1"
    :
    : "r"(0), "r"(0)
  );

  return 0;
}


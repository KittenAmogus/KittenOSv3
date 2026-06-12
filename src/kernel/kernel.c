#include "stdio.h"
#include "stdint.h"

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "pic/pic.h"

const char * const _hello_message = (
  "Hello, User!\n"
  "This is operating system, written with C and NASM, using\n"
  "- The little book about OsDev\n"
  "By @kittenamogus\n"
  "With help from Google AI\n"
);

void clearScreen(void) {
  cursorPos(0, 0);
  for (int a=0; a<(80 * 25); ++a) {
    putc(0);
  }
  cursorPos(0, 0);
}

// Entry
int main(void) {
  gdt_init();
  idt_init();
  pic_remap();
  asm volatile ("sti");

  vgaAttrs(7, 1);
  clearScreen();

  // Main
  puts(_hello_message);

  return 0;
}


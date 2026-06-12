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
  "By @kittenamogus\n"
  "With help from Google AI\n"
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

void printHex(unsigned int x) {
  putc('0');
  putc('x');

  for (uint8_t i=0; i<8; i++) {
    uint8_t c = (x >> (28 - (i << 2)) & 0x0F);

    if (c < 10) {
      c += '0';
    }
    else {
      c += 'A' - 10;
    }

    putc(c);
  }
}

// Entry
int main(struct mboot_info *mbi) {
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

  puts("Available memory:");
  printHex(ramSize);
  putc('\n');

  char *a = malloc(1 << 20);  // Alloc 1MB
  if (a == NULL) return 0xAAAAAAAA;
  puts("Allocated 1MB (A)");

  char *b = malloc(1 << 20);  // Another 1MB
  if (b == NULL) return 0xBBBBBBBB;
  puts("Allocated 1MB (B)");

  free(a);
  puts("Freed 1MB (A)");
  free(b);
  puts("Freed 1MB (B)");

  return 0;
}


#ifndef KERNEL_H
#define KERNEL_H

#include "stdint.h"

// Prints after boot
const char * const _hello_message = (
  "Hello, User!\n"
  "This is operating system, written with C and NASM, using\n"
  "- The little book about OsDev\n"
  "By @kittenamogus\n" "With help from Google AI\n"
  "No, not vibecoded, it was help with (GDT,IDT,PIC) and architecture\n"
);

// Multiboot data
typedef struct {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
} __attribute__((packed)) mboot_info;

// Entry
uint32_t kmain(mboot_info *mbi);

#endif // KERNEL_H


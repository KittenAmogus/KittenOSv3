#include "idt.h"
#include "stdint.h"

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr ip;

extern void irq1_wrapper(void);

void idt_set_gate(unsigned char num, unsigned int base, unsigned short selector, unsigned char flags) {
  idt[num].base_low  = (base & 0xFFFF);
  idt[num].base_high = (base >> 16) & 0xFFFF;

  idt[num].selector  = selector;
  idt[num].always0   = 0;
  idt[num].flags     = flags;
}

void idt_init(void) {
  ip.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
  ip.base  = (unsigned int)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idt_set_gate(i, 0, 0, 0);
  }

  // Unlock keyboard
  idt_set_gate(33, (unsigned int)irq1_wrapper, 0x08, 0x8E);

  load_idt((unsigned int)&ip);
}


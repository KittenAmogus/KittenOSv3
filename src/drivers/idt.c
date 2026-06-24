#include <drivers/idt.h>

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr ip;

void idt_set_gate(unsigned char num, unsigned int base, unsigned short selector,
                  unsigned char flags) {
  idt[num].base_low = (base & 0xFFFF);
  idt[num].base_high = (base >> 16) & 0xFFFF;

  idt[num].selector = selector;
  idt[num].always0 = 0;
  idt[num].flags = flags;
}

void idt_init(void) {
  ip.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
  ip.base = (unsigned int)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idt_set_gate(i, 0, 0, 0);
  }

  idt_flush((unsigned int)&ip);
}

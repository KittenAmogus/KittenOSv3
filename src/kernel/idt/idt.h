#ifndef IDT_H
#define IDT_H

#include "stdint.h"

struct idt_entry {
  uint16_t base_low;
  uint16_t selector;
  uint8_t always0;
  uint8_t flags;
  uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256

void load_idt(unsigned int idt_pointer);
void idt_init(void);

#endif // IDT_H


#include <drivers/idt.h>
#include <drivers/io.h>
#include <stddef.h>

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr ip;

extern void isr_keyboard(void);
extern void irq_handler_kbd(void);

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

  idt_set_gate(33, (size_t)isr_keyboard, 0x08, 0x8E);

  idt_flush((unsigned int)&ip);
}

void idt_handler(registers_t *regs) {
  switch (regs->int_no) {
  case 33: {
    irq_handler_kbd();
    break;
  }
  }

  if (regs->int_no >= 40) {
    outb(0xA0, 0x20); // Slave
  }
  outb(0x20, 0x20); // Master
}

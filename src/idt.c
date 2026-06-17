#include <io.h>
#include <idt.h>
#include <string.h>

static idt_entry_t  _IDT[256];
static idt_ptr_t    _idt_ptr;

extern void idt_flush(uint32_t ptra);

// Interrupts
extern void isr_timer(void);
extern void isr_keyboard(void);

// Handlers
extern void keyboard_handler(void);

static void _idt_setentry(int i, uint32_t base) {
  _IDT[i].offset_low  = base & 0xFFFF;
  _IDT[i].selector    = 0x08; // GDT Ring 0
  _IDT[i].zero        = 0x00; // Always 0
  _IDT[i].attr        = 0x8E; // Gate Ring 0
  _IDT[i].offset_high = (base >> 16) & 0xFFFF;
}

static void _pic_remap(void) {
  // ICW1 Init
  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  // ICW2 Vector offset
  outb(0x21, 0x20); // 32-39
  outb(0xA1, 0x28); // 40-47

  // ICW3 Link
  outb(0x21, 0x04);
  outb(0xA1, 0x02);

  // ICW4 Work mode
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  // Unlock
  outb(0x21, 0x00);
  outb(0xA1, 0x00);
}

void idt_init(void) {
  memset(&_IDT, 0, 256 * sizeof(idt_entry_t));

  // Entries
  _idt_setentry(32, (uint32_t)isr_timer);
  _idt_setentry(33, (uint32_t)isr_keyboard);

  _pic_remap(); // Offset to 32+

  // Flush
  _idt_ptr.limit  = ((sizeof(idt_entry_t) * 256) - 1);
  _idt_ptr.base   = (uint32_t)&_IDT;
  idt_flush((uint32_t)&_idt_ptr);

  __asm__ volatile ("sti"); // Enable interrupts
}

void idt_handler(registers_t *regs) {
  switch (regs->int_no) {
    case 32: {
      /* timer_handler */
      break;
    }

    case 33: {
      keyboard_handler();
      break;
    }

    default: {
      break;
    }
  }

  if (regs->int_no >= 40) {
    outb(0xA0, 0x20); // EOI for Slave PIC
  }
  outb(0x20, 0x20); // EOI for Master PIC
}


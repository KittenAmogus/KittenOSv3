#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>

typedef struct {
  uint16_t  offset_low;
  uint16_t  selector;
  uint8_t   zero;
  uint8_t   attr;
  uint16_t  offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idt_ptr_t;

typedef struct {
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, error_code;
  uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) registers_t;

void idt_init(void);

#endif // _IDT_H


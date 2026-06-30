#ifndef IDT_H
#define IDT_H

#include <stdint.h>

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

typedef struct registers {
  uint32_t ds; // Сегмент данных (наш push eax/ds)
  uint32_t edi, esi, ebp, useless_esp, ebx, edx, ecx,
      eax;                   // Хак: строгое аппаратное соответствие pusha!
  uint32_t int_no, err_code; // То, что затолкнули перед jmp irq_common_stub
  uint32_t eip, cs, eflags, useresp,
      ss; // Автоматически заталкивает сам процессор x86
} __attribute__((packed))
registers_t; // Обязательно упаковываем, чтобы GCC не добавил паддинги!

#define IDT_ENTRIES 256

void idt_flush(unsigned int idt_pointer);
void idt_init(void);

#endif // IDT_H

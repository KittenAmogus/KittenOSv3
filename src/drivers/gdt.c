#include <drivers/gdt.h>

struct gdt_entry gdt[3];
struct gdt_ptr gp;

void gdt_set_gate(int num, unsigned int base, unsigned int limit,
                  unsigned char access, unsigned char gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = (limit >> 16) & 0x0F;

  gdt[num].granularity |= gran & 0xF0;
  gdt[num].access = access;
}

void gdt_init(void) {
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (unsigned int)&gdt;

  // Zero descr
  gdt_set_gate(0, 0, 0, 0, 0);

  // Code segment
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

  // Data segment
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  // Flush
  gdt_flush((unsigned int)&gp);
}

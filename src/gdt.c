#include <gdt.h>

static gdt_entry_t _GDT[3];
static gdt_ptr_t _gdt_ptr;

extern void gdt_flush(uint32_t ptra);

static void _gdt_setentry(uint16_t ll, uint16_t bl, uint16_t bm, uint8_t ac, uint8_t gn, uint8_t bh) {
  static uint8_t i = 0;
  _GDT[i].limit_low   = ll;
  _GDT[i].base_low    = bl;
  _GDT[i].base_middle = bm;
  _GDT[i].access      = ac;
  _GDT[i].gran        = gn;
  _GDT[i].base_high   = bh;
  ++i;
}

void gdt_init(void) {
  // Null-descriptor
  _gdt_setentry(0, 0, 0, 0, 0, 0);

  // Code
  _gdt_setentry(0xFFFF, 0, 0, 0x9A, 0xCF, 0);

  // Data
  _gdt_setentry(0xFFFF, 0, 0, 0x92, 0xCF, 0);

  // Pointer
  _gdt_ptr.limit  = (sizeof(gdt_entry_t) * 3) - 1;
  _gdt_ptr.base   = (uint32_t)&_GDT;

  gdt_flush((uint32_t)&_gdt_ptr);
}


#ifndef _GDT_H
#define _GDT_H

#include <io.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint16_t  limit_low;
  uint16_t  base_low;
  uint8_t   base_middle;
  uint8_t   access;
  uint8_t   gran;
  uint8_t   base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
  uint16_t  limit;
  uint32_t  base;
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);

#endif // _GDT_H


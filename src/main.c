#include <errno.h>
#include <stdint.h>

#include <drivers/gdt.h>
#include <drivers/idt.h>
#include <drivers/pic.h>

static inline void _kernel_init(uint32_t _) {
  gdt_init();
  idt_init();
  pic_remap();
}

int kmain(uint32_t magic, uint32_t mboot_addr) {
  // Check boot info
  if (magic != 0x2BADB002)
    return EINVAL;
  if (mboot_addr == 0)
    return EINVAL;

  // Init kernel
  _kernel_init(mboot_addr);

  /* Here goes main code */

  return SUCCESS;
}

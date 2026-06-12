#include "pic.h"
#include "drivers/io/io.h"

void pic_remap(void) {
  // ICW1
  outb(PIC1_COMMAND, ICW1_INIT);
  outb(PIC2_COMMAND, ICW1_INIT);

  // ICW2
  outb(PIC1_DATA, 0x20);
  outb(PIC2_DATA, 0x28);

  // ICW3
  outb(PIC1_DATA, 0x04);
  outb(PIC2_DATA, 0x02);

  // ICW4
  outb(PIC1_DATA, 0x01);
  outb(PIC2_DATA, 0x01);

  // Mask
  // IRQ1 - Keyboard
  // 0xFD = 0b11111101
  outb(PIC1_DATA, 0xFD);  // Enable (1 << 1)
  outb(PIC2_DATA, 0xFF);  // Disable everything
}



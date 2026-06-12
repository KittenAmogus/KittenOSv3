global irq1_wrapper
extern keyboard_handler

irq1_wrapper:
  pusha ; Save E*X

  call keyboard_handler ; C handler

  ; End of Interrupt
  mov al, 0x20
  out 0x20, al

  popa  ; Load E*X
  iret  ; Return from interrupt


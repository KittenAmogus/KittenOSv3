MAGIC EQU 0x1BADB002      ; GRUB multiboot header
FLAGS EQU 0               ; Multiboot flags
CHECK EQU -(MAGIC+FLAGS)  ; Checksum (FLAGS+MAGIC+CHECK = 0)
STACKSIZE EQU 16384       ; 16KB stack

section .text
align 4 ; Aligned multiboot header
  dd  MAGIC
  dd  FLAGS
  dd  CHECK

global _start
extern kmain

_start:
  mov esp,  kernel_stack_start  ; Stack start
  push  ebx                     ; Multiboot data

  call kmain  ; Kernel entry

  ; Halt processor after exit
  cli
  hlt

  jmp $ ; Emulator protection

section .bss
kernel_stack_end:
  resb  STACKSIZE
kernel_stack_start:


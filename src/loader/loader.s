global loader ; Entry for GRUB

MAGIC     EQU 0x1BADB002  ; Grub magic number
FLAGS     EQU 0x00        ; Multiboot flags
CHECKSUM  EQU -MAGIC      ; Magic + Flags + Checksum = 0

KERNEL_STACK_SIZE EQU 4096  ; Stack size

section .text ; Code section
align 4       ; 4 byte aligned
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

extern kmain ; Kernel entry

; Entry
loader:
  mov esp,  kernel_stack + KERNEL_STACK_SIZE ; Set up stack
 
  push eax
  push ebx  ; Multiboot data to main
  call kmain

; Loop 4rvr
.loop:
  jmp .loop


section .bss
align 4

kernel_stack:
  resb KERNEL_STACK_SIZE


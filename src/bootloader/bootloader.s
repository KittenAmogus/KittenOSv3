MAGIC EQU 0x1BADB002
FLAGS EQU 0x00000000
CHECK EQU -(MAGIC+FLAGS)

STACK_SIZE  EQU 0x2000  ; 8KB of stack


section .text
align 4 ; 4byte alignment

; Multiboot header for GRUB
_header:
  dd  MAGIC
  dd  FLAGS
  dd  CHECK

global _start ; GRUB entry
extern kmain  ; Kernel entry

_start:
  mov esp,  stack_end ; Setup stack

  push  ebx ; multiboot info ptr
  push  eax ; Success boot magic (0x2BADB002)
  call kmain  ; Kernel main

  hlt   ; Stop CPU
  jmp $ ; If halt didn't work


; Stack
section .bss
stack_start:
  resb  STACK_SIZE
stack_end:


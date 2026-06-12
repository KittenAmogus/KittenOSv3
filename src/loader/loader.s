global loader ; Entry for GRUB

MAGIC     EQU 0x1BADB002  ; Grub magic number
FLAGS     EQU 0x00        ; Multiboot flags
CHECKSUM  EQU -MAGIC      ; Magic + Flags + Checksum = 0

section .text ; Code section
align 4       ; 4 byte aligned
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

; Entry
loader:
  mov eax,  0xCAFEBABE  ; Debug

; Loop 4rvr
.loop:
  jmp .loop


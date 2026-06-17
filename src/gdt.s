section .text
align 4

global gdt_flush
gdt_flush:
  push ebp
  mov ebp,  esp

  mov eax,  [ebp + 8]   ; Get address of _gdt_ptr from arguments
  lgdt [eax]            ; Load new GDT pointer into CPU register

  ; Reload data segment registers with Kernel Data Descriptor (0x10)
  mov ax,  0x10         ; 0x10 is offset of third entry in GDT (2 * 8)
  mov ds,  ax
  mov es,  ax
  mov fs,  ax
  mov gs,  ax
  mov ss,  ax           ; Reload stack segment as well

  ; Far jump to reload Code Segment register (CS) with Kernel Code Descriptor (0x08)
  ; 0x08 is offset of second entry in GDT (1 * 8)
  jmp 0x08:.reload_cs

.reload_cs:
  mov esp,  ebp
  pop ebp
  ret


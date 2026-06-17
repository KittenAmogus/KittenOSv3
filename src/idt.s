section .text
align 4

; ==============================================================================
; void idt_flush(uint32_t ptr_addr);
; Loads IDT pointer into CPU
; ==============================================================================
global idt_flush
idt_flush:
  push ebp
  mov ebp,  esp

  mov eax,  [ebp + 8]   ; Get address of idt_ptr from arguments
  lidt [eax]            ; Load IDT pointer into CPU register

  mov esp,  ebp
  pop ebp
  ret

; ==============================================================================
; ISR Stubs for Hardware Interrupts (IRQs)
; ==============================================================================
global isr_timer
global isr_keyboard

isr_timer:
  push 0                ; Fake error code
  push 32               ; Interrupt number (IRQ 0 remapped to 32)
  jmp irq_common_stub

isr_keyboard:
  push 0                ; Fake error code
  push 33               ; Interrupt number (IRQ 1 remapped to 33)
  jmp irq_common_stub

; ==============================================================================
; Common Interrupt Stub (C ABI Bridge)
; ==============================================================================
extern idt_handler      ; Our future C function dispatching interrupts

irq_common_stub:
  pusha                 ; Push EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX

  mov ax,  0x10         ; Reload data segments with Kernel Data Descriptor
  mov ds,  ax
  mov es,  ax
  mov fs,  ax
  mov gs,  ax

  push esp              ; Push pointer to registers stack as argument
  call idt_handler      ; Jump into comfortable C code!
  add esp,  4           ; Clean argument from stack

  popa                  ; Restore all registers
  add esp,  8           ; Clean error code and interrupt number from stack
  iret                  ; Interrupt return (restores EIP, CS, EFLAGS)

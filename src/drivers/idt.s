section .text
global idt_flush

idt_flush:
  mov eax,  [esp + 4] ; Get IDT pointer
  lidt  [eax]         ; Load IDT
  ret


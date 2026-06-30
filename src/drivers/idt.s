section .text
align 4

global idt_flush
idt_flush:
  mov eax,  [esp + 4] ; Get IDT pointer
  lidt  [eax]         ; Load IDT
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
  pusha                 ; 1. Заталкиваем 8 общих регистров (eax, ecx...)

  ; СОХРАНЯЕМ ОРИГИНАЛЬНЫЙ СЕГМЕНТ ДАННЫХ В СТЕК:
  xor eax, eax          ; Очищаем eax
  mov ax, ds            ; Берем текущий сегмент данных
  push eax              ; Заталкиваем его в стек! Теперь Си-структура совпадет байт-в-байт!

  ; Переключаемся на сегменты ядра
  mov ax,  0x10         
  mov ds,  ax
  mov es,  ax
  mov fs,  ax
  mov gs,  ax

  push esp              ; Передаем указатель на структуру registers_t как аргумент
  call idt_handler      ; Прыгаем в Си
  add esp,  4           ; Чистим аргумент

  ; ВОССТАНАВЛИВАЕМ СЕГМЕНТЫ ПРИ ВЫХОДЕ:
  pop eax               ; Вытряхиваем сохраненный сегмент из стека
  mov ds, ax            ; Восстанавливаем оригинальный DS процессора
  mov es, ax
  mov fs, ax
  mov gs, ax

  popa                  ; Восстанавливаем общие регистры
  add esp,  8           ; Перешагиваем через int_no и err_code
  iret                  ; Чистый выход

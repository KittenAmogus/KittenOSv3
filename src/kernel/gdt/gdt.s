global gdt_flush
gdt_flush:
  mov eax, [esp + 4]  ; Берем указатель на GDT из стека
  lgdt [eax]          ; Загружаем таблицу в процессор [2]

  ; Update CS
  jmp 0x08:.flush_cs  ; 0x08 — это смещение сегмента кода в нашей GDT [3]

.flush_cs:
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  ret


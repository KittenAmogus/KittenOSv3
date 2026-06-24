section .text

global outsw  ; Output StringWord
global outb ; Output byte
global outw ; Output word(2 byte)

global insw   ; Input StringWord(512 byte)
global inb  ; Input byte

outsw:
  push esi            ; Save ESI
  mov dx, [esp + 8]   ; Port
  mov esi, [esp + 12] ; Buffer address
  mov ecx, [esp + 16] ; StringWord count
  cld       ; Forward
  rep outsw ; Repead sending (ESI->DX) ECX times
  pop esi   ; Load ESI
  ret

outb:
  mov al, [esp + 8]
  mov dx, [esp + 4]
  out dx, al
  ret

outw:
  mov ax, [esp + 8]
  mov dx, [esp + 4]
  out dx, ax
  ret

insw:
  push edi            ; Save EDI
  mov dx, [esp + 8]   ; Port
  mov edi, [esp + 12] ; Buffer address
  mov ecx, [esp + 16] ; StringWord count
  cld       ; Forward
  rep insw  ; Repeat reading (DX->EDI) ECX times
  pop edi   ; Load EDI
  ret

inb:
  mov dx, [esp + 4]
  in  al, dx
  ret

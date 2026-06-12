global outb ; Output byte
global outw ; Output word(2 byte)
global inb  ; Input byte

; [ESP + 8] - Data (OUT)
; [ESP + 4] - Port (I/O)

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

inb:
  mov dx, [esp + 4]
  in  al, dx
  ret


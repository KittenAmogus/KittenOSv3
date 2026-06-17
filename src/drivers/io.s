section .text
align 4

global outb
outb:
  push ebp
  mov ebp,  esp

  mov edx,  [ebp + 8]   ; edx = port
  mov eax,  [ebp + 12]  ; al = value
  out dx,  al

  mov esp,  ebp
  pop ebp
  ret

global inb
inb:
  push ebp
  mov ebp,  esp

  xor eax,  eax         ; Clear EAX to ensure upper bits are zero
  mov edx,  [ebp + 8]   ; edx = port
  in al,  dx            ; Read byte into AL

  mov esp,  ebp
  pop ebp
  ret

global outsw
outsw:
  push ebp
  mov ebp,  esp
  push esi              ; Save ESI for C ABI compliance

  mov edx,  [ebp + 8]   ; edx = port
  mov esi,  [ebp + 12]  ; esi = source memory address
  mov ecx,  [ebp + 16]  ; ecx = words count
  cld                   ; Forward direction

  rep outsw             ; Send data array to port

  pop esi               ; Restore ESI
  mov esp,  ebp
  pop ebp
  ret

global insw
insw:
  push ebp
  mov ebp,  esp
  push edi              ; Save EDI for C ABI compliance

  mov edx,  [ebp + 8]   ; edx = port
  mov edi,  [ebp + 12]  ; edi = destination memory address
  mov ecx,  [ebp + 16]  ; ecx = words count
  cld                   ; Forward direction

  rep insw              ; Read data array from port

  pop edi               ; Restore EDI
  mov esp,  ebp
  pop ebp
  ret

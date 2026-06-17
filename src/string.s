section .text
align 4

global memcpy
memcpy:
  push ebp
  mov ebp,  esp
  push esi              ; Save registers for C ABI compliance
  push edi

  mov edi,  [ebp + 8]   ; edi = dest
  mov esi,  [ebp + 12]  ; esi = src
  mov ecx,  [ebp + 16]  ; ecx = n (bytes count)

  mov eax,  edi         ; Return value must be dest address
  cld                   ; Clear direction flag for forward copy

  mov edx,  ecx         ; Save original count to calculate remainder
  shr ecx,  2           ; Divide by 4 to get DWORD count
  rep movsd             ; Copy data 4 bytes at a time

  mov ecx,  edx         ; Restore original count
  and ecx,  3           ; Get remainder bytes (0..3)
  rep movsb             ; Copy remaining bytes 1 byte at a time

  pop edi               ; Restore registers
  pop esi
  mov esp,  ebp
  pop ebp
  ret

global memset
memset:
  push ebp
  mov ebp,  esp
  push edi              ; Save register for C ABI compliance

  mov edi,  [ebp + 8]   ; edi = s
  mov eax,  [ebp + 12]  ; eax = c (int converted to byte)
  mov ecx,  [ebp + 16]  ; ecx = n (bytes count)

  mov edx,  edi         ; Save original s address for return value
  and eax,  0xFF        ; Mask to get pure 8-bit byte in AL

  mov ah,  al           ; Duplicate AL byte into AH (AX = 0xCC77)
  mov cx,  ax           ; Save AX word temporarily into CX register
  shl eax,  16          ; Shift EAX left (EAX = 0xCC770000)
  mov ax,  cx           ; Restore AX word (EAX = 0xCC77CC77)

  mov ecx,  [ebp + 16]  ; Reload original n into ECX
  cld                   ; Clear direction flag for forward fill

  mov edx,  ecx         ; Save original count to calculate remainder
  shr ecx,  2           ; Divide by 4 to get DWORD count
  rep stosd             ; Fill memory 4 bytes at a time

  mov ecx,  edx         ; Restore original count
  and ecx,  3           ; Get remainder bytes (0..3)
  rep stosb             ; Fill remaining bytes 1 byte at a time

  mov eax,  edx         ; Return value must be s address
  pop edi               ; Restore register
  mov esp,  ebp
  pop ebp
  ret


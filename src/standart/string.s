section .text
align 4

global memset
global memcpy
global strcpy
global strcat

memset:
  push  ebp
  mov ebp,  esp

  ; Save edi
  push  edi

  mov edi,  [ebp + 8]   ; Destination
  mov eax,  [ebp + 12]  ; Value 
  mov edx,  [ebp + 16]  ; Count

  ; Copy al to all eax bytes
  mov ah, al    ; ah = al
  mov cx, ax    ; cx = ax
  shl eax,  16  ; eax = ax??
  mov ax, cx    ; eax = axax

  ; Dword count
  mov ecx,  edx ; Copy to ecx
  shr ecx,  2   ; Dword count
  rep stosd     ; Copy dwords

  ; Byte count
  mov ecx,  edx ; Load count
  and ecx,  3   ; Byte count

  jz .end       ; 0 bytes
  rep stosb     ; Copy bytes

.end:
  pop edi ; Load edi
  mov esp,  ebp
  pop ebp
  ret

memcpy:
  push  ebp
  mov ebp,  esp

  ; Save edi & esi
  push  edi
  push  esi

  mov edi,  [ebp + 8]   ; Destination
  mov esi,  [ebp + 12]  ; Source
  mov edx,  [ebp + 16]  ; Count

  ; Dword count
  mov ecx,  edx ; Copy to ecx
  shr ecx,  2   ; Dword count
  rep movsd     ; Move dwords

  ; Byte count
  mov ecx,  edx ; Load count
  and ecx,  3   ; Byte count

  jz .end       ; 0 bytes
  rep movsb     ; Move bytes

.end:
  pop esi ; Load esi
  pop edi ; Load edi
  mov esp,  ebp
  pop ebp
  ret

strcpy:
  push  ebp
  mov ebp,  esp

  ; Save edi & esi
  push  edi
  push  esi

  mov edi,  [ebp + 8]   ; Destination
  mov esi,  [ebp + 12]  ; Source
  push  edi ; Save *dest

.loop:
  mov al, byte [esi]
  mov byte [edi],  al

  ; *ecx == 0
  test  al, al
  jz .end

  inc edi
  inc esi
  jmp .loop

.end:
  pop eax ; Return *dest
  pop esi ; Load esi
  pop edi ; Load edi
  mov esp,  ebp
  pop ebp
  ret

strcat:
  push  ebp
  mov ebp,  esp

  ; Save edi & esi
  push  edi
  push  esi

  mov edi,  [ebp + 8]   ; Destination
  mov esi,  [ebp + 12]  ; Source
  push  edi ; Save *dest

.findloop:
  mov al, byte [edi]
  test  al, al
  jz  .cpyloop

  inc edi
  jmp .findloop

.cpyloop:
  mov al, byte [esi]
  mov byte [edi],  al

  ; *ecx == 0
  test  al, al
  jz .end

  inc edi
  inc esi
  jmp .cpyloop

.end:
  pop eax ; Return *dest
  pop esi ; Load esi
  pop edi ; Load edi
  mov esp,  ebp
  pop ebp
  ret



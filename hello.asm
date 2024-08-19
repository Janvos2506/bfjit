use32

; Assume WriteFile is in [esp+4]
; Handle is in [esp+8]
; String (buffer) is in [esp+12]
; Length is in [esp+16]

; First, reserve space for lpNumberOfBytesWritten (usually done on stack)
sub esp, 4              ; allocate space for lpNumberOfBytesWritten
mov dword [esp], 0      ; initialize it to 0 or a variable can be used instead

push 0                  ; lpOverlapped (NULL, as we're not using it)
lea eax, [esp]          ; get the address of lpNumberOfBytesWritten
push eax                ; lpNumberOfBytesWritten
push dword [esp + 28]   ; nNumberOfBytesToWrite
push dword [esp + 28]   ; lpBuffer
push dword [esp + 28]   ; hFile

mov eax, [esp + 28] ; move the address of WriteFile into eax
call eax                ; Call WriteFile
pop eax                 ; Clean up the stack
ret
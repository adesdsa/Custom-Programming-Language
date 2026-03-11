; picoC arraylib (Windows x86, cdecl)
; Args on stack: [esp+4]=base, [esp+8]=size, [esp+12]=value
BITS 32

global _arr_find_int
global _arr_find_first_of_int
global _arr_find_last_of_int
global _arr_find_str
global _arr_find_first_of_str
global _arr_find_last_of_str
global _arr_find_float
global _arr_find_first_of_float
global _arr_find_last_of_float
global _arr_find_double
global _arr_find_first_of_double
global _arr_find_last_of_double
global _arr_find_char
global _arr_find_first_of_char
global _arr_find_last_of_char

extern _str_cmp

section .text

; _arr_find_int(arr: int*, size: int32, value: int32) -> int32
; picoC layout: arr[i] at base - i*4
; cdecl: callee must preserve ebx, esi, edi
_arr_find_int:
_arr_find_first_of_int:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    ; args: [ebp+8]=base, [ebp+12]=size, [ebp+16]=value (after our pushes)
    xor eax, eax
    mov ecx, [ebp+8]
    mov edx, [ebp+12]
    mov ebx, [ebp+16]
    cmp edx, 0
    jle .find_int_npos
.find_int_loop:
    cmp eax, edx
    jge .find_int_npos
    mov esi, ecx
    mov edi, eax
    imul edi, 4
    sub esi, edi
    cmp [esi], ebx
    je .find_int_ret
    inc eax
    jmp .find_int_loop
.find_int_ret:
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.find_int_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; _arr_find_last_of_int
_arr_find_last_of_int:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov ecx, [ebp+8]
    mov edx, [ebp+12]
    mov ebx, [ebp+16]
    mov eax, edx
    dec eax
.findlast_int_loop:
    cmp eax, 0
    jl .findlast_int_npos
    mov esi, ecx
    mov edi, eax
    imul edi, 4
    sub esi, edi
    cmp [esi], ebx
    je .findlast_int_ret
    dec eax
    jmp .findlast_int_loop
.findlast_int_ret:
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.findlast_int_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; _arr_find_str(arr: char**, size: int32, value: char*) -> int32
; element size 4 (pointer)
_arr_find_str:
_arr_find_first_of_str:
    push ebx
    push esi
    push edi
    xor eax, eax
    mov ecx, [esp+16]
    mov edx, [esp+20]
    mov ebx, [esp+24]
.find_str_loop:
    cmp eax, edx
    jge .find_str_npos
    push eax
    push ecx
    push edx
    push ebx
    mov esi, ecx
    mov edi, eax
    imul edi, 4
    sub esi, edi
    push dword [ebx]
    push dword [esi]
    call _str_cmp
    add esp, 8
    pop ebx
    pop edx
    pop ecx
    pop edi
    test eax, eax
    jz .find_str_ret
    mov eax, edi
    inc eax
    jmp .find_str_loop
.find_str_ret:
    mov eax, edi
    pop edi
    pop esi
    pop ebx
    ret
.find_str_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    ret

; _arr_find_last_of_str
_arr_find_last_of_str:
    push ebx
    push esi
    push edi
    mov ecx, [esp+16]
    mov edx, [esp+20]
    mov ebx, [esp+24]
    mov eax, edx
    dec eax
.findlast_str_loop:
    cmp eax, 0
    jl .findlast_str_npos
    push eax
    push ecx
    push edx
    push ebx
    mov esi, ecx
    mov edi, eax
    imul edi, 4
    sub esi, edi
    push dword [ebx]
    push dword [esi]
    call _str_cmp
    add esp, 8
    pop ebx
    pop edx
    pop ecx
    pop edi
    test eax, eax
    jz .findlast_str_ret
    mov eax, edi
    dec eax
    jmp .findlast_str_loop
.findlast_str_ret:
    mov eax, edi
    pop edi
    pop esi
    pop ebx
    ret
.findlast_str_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    ret

; _arr_find_float and _arr_find_double: on win32 picoC uses float32 (4 bytes) for double
; arr: float*, size: int32, value: float (4 bytes). [esp+4]=base, [esp+8]=size, [esp+12]=value
_arr_find_float:
_arr_find_first_of_float:
_arr_find_double:
_arr_find_first_of_double:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    ; args: [ebp+8]=base, [ebp+12]=size, [ebp+16]=value
    xor eax, eax
    mov ecx, [ebp+8]
    mov edx, [ebp+12]
    mov ebx, [ebp+16]
.find_float_loop:
    cmp eax, edx
    jge .find_float_npos
    mov esi, ecx
    mov edi, eax
    imul edi, 4
    sub esi, edi
    cmp [esi], ebx
    je .find_float_ret
    inc eax
    jmp .find_float_loop
.find_float_ret:
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.find_float_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

_arr_find_last_of_float:
_arr_find_last_of_double:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov ecx, [ebp+8]
    mov edx, [ebp+12]
    mov ebx, [ebp+16]
    mov eax, edx
    dec eax
.findlast_float_loop:
    cmp eax, 0
    jl .findlast_float_npos
    mov esi, ecx
    mov edi, eax
    imul edi, 4
    sub esi, edi
    cmp [esi], ebx
    je .findlast_float_ret
    dec eax
    jmp .findlast_float_loop
.findlast_float_ret:
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.findlast_float_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; _arr_find_char(arr: char*, size: int32, value: char) -> int32
; value in byte at [esp+12]
_arr_find_char:
_arr_find_first_of_char:
    push ebx
    push esi
    push edi
    xor eax, eax
    mov ecx, [esp+16]
    mov edx, [esp+20]
    mov bl, [esp+24]
.find_char_loop:
    cmp eax, edx
    jge .find_char_npos
    mov esi, ecx
    sub esi, eax
    cmp [esi], bl
    je .find_char_ret
    inc eax
    jmp .find_char_loop
.find_char_ret:
    pop edi
    pop esi
    pop ebx
    ret
.find_char_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    ret

; _arr_find_last_of_char
_arr_find_last_of_char:
    push ebx
    push esi
    push edi
    mov ecx, [esp+16]
    mov edx, [esp+20]
    mov bl, [esp+24]
    mov eax, edx
    dec eax
.findlast_char_loop:
    cmp eax, 0
    jl .findlast_char_npos
    mov esi, ecx
    sub esi, eax
    cmp [esi], bl
    je .findlast_char_ret
    dec eax
    jmp .findlast_char_loop
.findlast_char_ret:
    pop edi
    pop esi
    pop ebx
    ret
.findlast_char_npos:
    mov eax, -1
    pop edi
    pop esi
    pop ebx
    ret

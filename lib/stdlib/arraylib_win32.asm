; picoC arraylib (Windows x86, cdecl)
; Args on stack: [esp+4]=base, [esp+8]=size, [esp+12]=value
BITS 32

global _arr_find_int
global _arr_find_first_of_int
global _arr_find_last_of_int
global _arr_find_str
global _arr_find_first_of_str
global _arr_find_last_of_str
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
_arr_find_int:
_arr_find_first_of_int:
    xor eax, eax
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov ebx, [esp+12]
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
    ret
.find_int_npos:
    mov eax, -1
    ret

; _arr_find_last_of_int
_arr_find_last_of_int:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov ebx, [esp+12]
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
    ret
.findlast_int_npos:
    mov eax, -1
    ret

; _arr_find_str(arr: char**, size: int32, value: char*) -> int32
; element size 4 (pointer)
_arr_find_str:
_arr_find_first_of_str:
    xor eax, eax
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov ebx, [esp+12]
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
    ret
.find_str_npos:
    mov eax, -1
    ret

; _arr_find_last_of_str
_arr_find_last_of_str:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov ebx, [esp+12]
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
    ret
.findlast_str_npos:
    mov eax, -1
    ret

; _arr_find_double(arr: double*, size: int32, value: double) -> int32
; cdecl: base, size, value_lo, value_hi. [esp+4]=base, [esp+8]=size, [esp+12]=value_lo, [esp+16]=value_hi
_arr_find_double:
_arr_find_first_of_double:
    xor eax, eax
    mov ecx, [esp+4]
    mov edx, [esp+8]
.find_double_loop:
    cmp eax, edx
    jge .find_double_npos
    mov esi, ecx
    mov edi, eax
    imul edi, 8
    sub esi, edi
    mov ebx, [esp+12]
    cmp [esi], ebx
    jne .find_double_next
    mov ebx, [esp+16]
    cmp [esi+4], ebx
    je .find_double_ret
.find_double_next:
    inc eax
    jmp .find_double_loop
.find_double_ret:
    ret
.find_double_npos:
    mov eax, -1
    ret

_arr_find_last_of_double:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov eax, edx
    dec eax
.findlast_double_loop:
    cmp eax, 0
    jl .findlast_double_npos
    mov esi, ecx
    mov edi, eax
    imul edi, 8
    sub esi, edi
    mov ebx, [esp+12]
    cmp [esi], ebx
    jne .findlast_double_next
    mov ebx, [esp+16]
    cmp [esi+4], ebx
    je .findlast_double_ret
.findlast_double_next:
    dec eax
    jmp .findlast_double_loop
.findlast_double_ret:
    ret
.findlast_double_npos:
    mov eax, -1
    ret

; _arr_find_char(arr: char*, size: int32, value: char) -> int32
; value in byte at [esp+12]
_arr_find_char:
_arr_find_first_of_char:
    xor eax, eax
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov bl, [esp+12]
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
    ret
.find_char_npos:
    mov eax, -1
    ret

; _arr_find_last_of_char
_arr_find_last_of_char:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    mov bl, [esp+12]
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
    ret
.findlast_char_npos:
    mov eax, -1
    ret
</think>

<｜tool▁calls▁begin｜><｜tool▁call▁begin｜>
StrReplace
; picoC arraylib (Linux x86_64, SysV ABI)
; rdi=arg1 (base), rsi=arg2 (size), rdx=arg3 (value)
BITS 64

global arr_find_int
global arr_find_first_of_int
global arr_find_last_of_int
global arr_find_str
global arr_find_first_of_str
global arr_find_last_of_str
global arr_find_double
global arr_find_first_of_double
global arr_find_last_of_double
global arr_find_char
global arr_find_first_of_char
global arr_find_last_of_char

extern str_cmp

section .text

; arr_find_int(arr: int*, size: int64, value: int64) -> int64
; picoC layout: arr[i] at base - i*8 (stack grows down)
arr_find_int:
arr_find_first_of_int:
    xor rax, rax
.find_int_loop:
    cmp rax, rsi
    jge .find_int_npos
    mov r9, rdi
    mov r10, rax
    imul r10, 8
    sub r9, r10
    cmp [r9], rdx
    je .find_int_ret
    inc rax
    jmp .find_int_loop
.find_int_ret:
    ret
.find_int_npos:
    mov rax, -1
    ret

; arr_find_last_of_int(arr: int*, size: int64, value: int64) -> int64
arr_find_last_of_int:
    mov rax, rsi
    dec rax
.findlast_int_loop:
    cmp rax, 0
    jl .findlast_int_npos
    mov r9, rdi
    mov r10, rax
    imul r10, 8
    sub r9, r10
    cmp [r9], rdx
    je .findlast_int_ret
    dec rax
    jmp .findlast_int_loop
.findlast_int_ret:
    ret
.findlast_int_npos:
    mov rax, -1
    ret

; arr_find_str(arr: char**, size: int64, value: char*) -> int64
arr_find_str:
arr_find_first_of_str:
    xor rax, rax
.find_str_loop:
    cmp rax, rsi
    jge .find_str_npos
    push rax
    push rdi
    push rsi
    push rdx
    mov r11, rdi
    mov r10, rax
    imul r10, 8
    sub r11, r10
    mov rdi, [r11]
    mov rsi, rdx
    call str_cmp
    pop rdx
    pop rsi
    pop rdi
    pop r9
    test eax, eax
    jz .find_str_ret
    mov rax, r9
    inc rax
    jmp .find_str_loop
.find_str_ret:
    mov rax, r9
    ret
.find_str_npos:
    mov rax, -1
    ret

; arr_find_last_of_str(arr: char**, size: int64, value: char*) -> int64
arr_find_last_of_str:
    mov rax, rsi
    dec rax
.findlast_str_loop:
    cmp rax, 0
    jl .findlast_str_npos
    push rax
    push rdi
    push rsi
    push rdx
    mov r11, rdi
    mov r10, rax
    imul r10, 8
    sub r11, r10
    mov rdi, [r11]
    mov rsi, rdx
    call str_cmp
    pop rdx
    pop rsi
    pop rdi
    pop r9
    test eax, eax
    jz .findlast_str_ret
    mov rax, r9
    dec rax
    jmp .findlast_str_loop
.findlast_str_ret:
    mov rax, r9
    ret
.findlast_str_npos:
    mov rax, -1
    ret

; arr_find_double(arr: double*, size: int64, value: double) -> int64
; SysV: rdi=ptr, rsi=size, rdx=value (as bits)
arr_find_double:
arr_find_first_of_double:
    xor rax, rax
.find_double_loop:
    cmp rax, rsi
    jge .find_double_npos
    mov r9, rdi
    mov r10, rax
    imul r10, 8
    sub r9, r10
    cmp [r9], rdx
    je .find_double_ret
    inc rax
    jmp .find_double_loop
.find_double_ret:
    ret
.find_double_npos:
    mov rax, -1
    ret

; arr_find_last_of_double(arr: double*, size: int64, value: double) -> int64
arr_find_last_of_double:
    mov rax, rsi
    dec rax
.findlast_double_loop:
    cmp rax, 0
    jl .findlast_double_npos
    mov r9, rdi
    mov r10, rax
    imul r10, 8
    sub r9, r10
    cmp [r9], rdx
    je .findlast_double_ret
    dec rax
    jmp .findlast_double_loop
.findlast_double_ret:
    ret
.findlast_double_npos:
    mov rax, -1
    ret

; arr_find_char(arr: char*, size: int64, value: char) -> int64
; value in dl
arr_find_char:
arr_find_first_of_char:
    xor rax, rax
.find_char_loop:
    cmp rax, rsi
    jge .find_char_npos
    mov r9, rdi
    sub r9, rax
    cmp [r9], dl
    je .find_char_ret
    inc rax
    jmp .find_char_loop
.find_char_ret:
    ret
.find_char_npos:
    mov rax, -1
    ret

; arr_find_last_of_char(arr: char*, size: int64, value: char) -> int64
arr_find_last_of_char:
    mov rax, rsi
    dec rax
.findlast_char_loop:
    cmp rax, 0
    jl .findlast_char_npos
    mov r9, rdi
    sub r9, rax
    cmp [r9], dl
    je .findlast_char_ret
    dec rax
    jmp .findlast_char_loop
.findlast_char_ret:
    ret
.findlast_char_npos:
    mov rax, -1
    ret

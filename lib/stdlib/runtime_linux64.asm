; picoC runtime: bounds checking (Linux x86_64, SysV ABI)
BITS 64

global bounds_check

extern fprintf
extern stderr
extern exit

section .data
    __bounds_check_msg db "array index out of bounds", 10, 0
    __bounds_check_fmt db "%s", 0

section .text

; bounds_check(index: int64, size: int64) -> void
; Aborts with exit(1) if index < 0 or index >= size
bounds_check:
    push rbp
    mov rbp, rsp
    ; rdi = index, rsi = size (SysV calling convention)
    cmp rdi, 0
    jl .fail
    cmp rdi, rsi
    jge .fail
    pop rbp
    ret
.fail:
    mov rdi, [rel stderr]
    lea rsi, [rel __bounds_check_fmt]
    lea rdx, [rel __bounds_check_msg]
    call fprintf
    mov edi, 1
    call exit
    ; never returns

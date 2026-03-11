; picoC runtime: bounds checking (Windows x86_64, MS ABI)
BITS 64

global bounds_check

extern printf
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
    sub rsp, 32
    ; rcx = index, rdx = size (Windows x64 calling convention)
    cmp rcx, 0
    jl .fail
    cmp rcx, rdx
    jge .fail
    add rsp, 32
    pop rbp
    ret
.fail:
    lea rcx, [rel __bounds_check_fmt]
    lea rdx, [rel __bounds_check_msg]
    call printf
    mov ecx, 1
    call exit
    ; never returns

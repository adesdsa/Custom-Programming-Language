; picoC runtime: bounds checking (Windows x86, cdecl)
BITS 32

global _bounds_check

extern _printf
extern _exit

section .data
    __bounds_check_msg db "array index out of bounds", 10, 0
    __bounds_check_fmt db "%s", 0

section .text

; bounds_check(index: int32, size: int32) -> void
; Aborts with exit(1) if index < 0 or index >= size
_bounds_check:
    push ebp
    mov ebp, esp
    ; [ebp+8] = index, [ebp+12] = size (cdecl)
    mov eax, [ebp+8]
    cmp eax, 0
    jl .fail
    cmp eax, [ebp+12]
    jge .fail
    pop ebp
    ret
.fail:
    push __bounds_check_msg
    push __bounds_check_fmt
    call _printf
    add esp, 8
    push 1
    call _exit
    ; never returns

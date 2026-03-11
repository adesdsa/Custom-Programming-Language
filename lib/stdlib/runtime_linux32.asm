; picoC runtime: bounds checking (Linux x86, cdecl)
BITS 32

global bounds_check

extern fprintf
extern stderr
extern exit

section .data
    __bounds_check_msg db "array index out of bounds", 10, 0
    __bounds_check_fmt db "%s", 0

section .text

; bounds_check(index: int32, size: int32) -> void
; Aborts with exit(1) if index < 0 or index >= size
bounds_check:
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
    push dword [stderr]
    call fprintf
    add esp, 12
    push 1
    call exit
    ; never returns

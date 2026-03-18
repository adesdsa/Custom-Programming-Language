; picoC standard library: math (Linux x86_64, SysV ABI)
; Doubles are passed as raw 64-bit integer values in rdi, rsi, rdx, rcx.
; Return value: raw double bits in rax.
BITS 64

global sqrt
global pow
global min
global max

section .text

; ---- sqrt(x: double) -> double ----
; rdi = raw bits of x
sqrt:
    push rbp
    mov rbp, rsp
    mov [rbp-8], rdi
    movsd xmm0, qword [rbp-8]
    sqrtsd xmm0, xmm0
    movq rax, xmm0
    pop rbp
    ret

; ---- pow(x: double, y: double) -> double ----
; rdi = raw bits of x, rsi = raw bits of y
; Computes x^y = 2^(y * log2(x)) using x87 FPU.
pow:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov [rbp-8], rdi
    mov [rbp-16], rsi

    ; Check x == 0
    movsd xmm0, qword [rbp-8]
    xorpd xmm1, xmm1
    ucomisd xmm0, xmm1
    jne .pow_nonzero
    jp .pow_nonzero
    ; x == 0: check y
    movsd xmm0, qword [rbp-16]
    ucomisd xmm0, xmm1
    jne .pow_return_zero
    jp .pow_return_zero
    ; x == 0 and y == 0 => return 1.0
    mov rax, 0x3FF0000000000000
    jmp .pow_done

.pow_return_zero:
    xor rax, rax
    jmp .pow_done

.pow_nonzero:
    ; Check if x < 0
    movsd xmm0, qword [rbp-8]
    xorpd xmm1, xmm1
    ucomisd xmm0, xmm1
    jae .pow_positive
    ; x < 0: negate x, compute |x|^y, then fix sign if y is odd integer
    mov rax, [rbp-8]
    btr rax, 63
    mov [rbp-24], rax
    ; Compute |x|^y
    fld qword [rbp-16]
    fld qword [rbp-24]
    fyl2x
    fld st0
    frndint
    fsub st1, st0
    fxch
    f2xm1
    fld1
    faddp
    fscale
    fstp st1
    fstp qword [rbp-24]
    ; Check if y is an integer
    fld qword [rbp-16]
    fld st0
    frndint
    fcomip st0, st1
    fstp st0
    jne .pow_neg_result
    ; y is integer, check if odd
    fld qword [rbp-16]
    fistp qword [rbp-32]
    mov rax, [rbp-32]
    test rax, 1
    jz .pow_neg_result_pos
    ; y is odd integer: negate result
    mov rax, [rbp-24]
    bts rax, 63
    jmp .pow_done

.pow_neg_result_pos:
    mov rax, [rbp-24]
    jmp .pow_done

.pow_neg_result:
    mov rax, [rbp-24]
    jmp .pow_done

.pow_positive:
    fld qword [rbp-16]
    fld qword [rbp-8]
    fyl2x
    fld st0
    frndint
    fsub st1, st0
    fxch
    f2xm1
    fld1
    faddp
    fscale
    fstp st1
    fstp qword [rbp-8]
    mov rax, [rbp-8]

.pow_done:
    add rsp, 32
    pop rbp
    ret

; ---- min(x: double, y: double) -> double ----
; rdi = raw bits of x, rsi = raw bits of y
min:
    push rbp
    mov rbp, rsp
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    movsd xmm0, qword [rbp-8]
    movsd xmm1, qword [rbp-16]
    minsd xmm0, xmm1
    movq rax, xmm0
    pop rbp
    ret

; ---- max(x: double, y: double) -> double ----
; rdi = raw bits of x, rsi = raw bits of y
max:
    push rbp
    mov rbp, rsp
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    movsd xmm0, qword [rbp-8]
    movsd xmm1, qword [rbp-16]
    maxsd xmm0, xmm1
    movq rax, xmm0
    pop rbp
    ret

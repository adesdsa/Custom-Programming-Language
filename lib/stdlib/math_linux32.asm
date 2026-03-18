; picoC standard library: math (Linux x86, cdecl)
; On 32-bit, doubles are stored as 32-bit floats (raw bits in dwords).
; cdecl: args on stack [ebp+8], [ebp+12], etc. Return in eax (raw float bits).
; Linux 32-bit: no underscore prefix.
BITS 32

global sqrt
global pow
global min
global max

section .text

; ---- sqrt(x: float) -> float ----
; [ebp+8] = raw bits of x
sqrt:
    push ebp
    mov ebp, esp
    movss xmm0, dword [ebp+8]
    sqrtss xmm0, xmm0
    movd eax, xmm0
    pop ebp
    ret

; ---- pow(x: float, y: float) -> float ----
; [ebp+8] = raw bits of x, [ebp+12] = raw bits of y
; Computes x^y = 2^(y * log2(x)) using x87 FPU.
pow:
    push ebp
    mov ebp, esp
    sub esp, 16

    ; Convert float args to double for FPU precision
    movss xmm0, dword [ebp+8]
    cvtss2sd xmm0, xmm0
    movsd qword [ebp-8], xmm0
    movss xmm0, dword [ebp+12]
    cvtss2sd xmm0, xmm0
    movsd qword [ebp-16], xmm0

    ; Check x == 0
    movss xmm0, dword [ebp+8]
    xorps xmm1, xmm1
    ucomiss xmm0, xmm1
    jne .pow_nonzero
    jp .pow_nonzero
    ; x == 0: check y
    movss xmm0, dword [ebp+12]
    ucomiss xmm0, xmm1
    jne .pow_return_zero
    jp .pow_return_zero
    ; 0^0 = 1.0
    mov eax, 0x3F800000
    jmp .pow_done

.pow_return_zero:
    xor eax, eax
    jmp .pow_done

.pow_nonzero:
    movss xmm0, dword [ebp+8]
    xorps xmm1, xmm1
    ucomiss xmm0, xmm1
    jae .pow_positive

    ; x < 0: compute |x|^y then fix sign
    mov eax, [ebp+8]
    and eax, 0x7FFFFFFF
    mov [ebp-4], eax
    ; Convert |x| to double
    movss xmm0, dword [ebp-4]
    cvtss2sd xmm0, xmm0
    movsd qword [ebp-8], xmm0

    fld qword [ebp-16]
    fld qword [ebp-8]
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
    fstp qword [ebp-8]
    ; Convert result back to float
    movsd xmm0, qword [ebp-8]
    cvtsd2ss xmm0, xmm0
    movd eax, xmm0
    ; Check if y is integer and odd
    fld qword [ebp-16]
    fld st0
    frndint
    fcomip st0, st1
    fstp st0
    jne .pow_done
    fld qword [ebp-16]
    fistp dword [ebp-4]
    test dword [ebp-4], 1
    jz .pow_done
    ; Negate result
    or eax, 0x80000000
    jmp .pow_done

.pow_positive:
    fld qword [ebp-16]
    fld qword [ebp-8]
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
    fstp qword [ebp-8]
    movsd xmm0, qword [ebp-8]
    cvtsd2ss xmm0, xmm0
    movd eax, xmm0

.pow_done:
    add esp, 16
    pop ebp
    ret

; ---- min(x: float, y: float) -> float ----
; [ebp+8] = raw bits of x, [ebp+12] = raw bits of y
min:
    push ebp
    mov ebp, esp
    movss xmm0, dword [ebp+8]
    movss xmm1, dword [ebp+12]
    minss xmm0, xmm1
    movd eax, xmm0
    pop ebp
    ret

; ---- max(x: float, y: float) -> float ----
; [ebp+8] = raw bits of x, [ebp+12] = raw bits of y
max:
    push ebp
    mov ebp, esp
    movss xmm0, dword [ebp+8]
    movss xmm1, dword [ebp+12]
    maxss xmm0, xmm1
    movd eax, xmm0
    pop ebp
    ret

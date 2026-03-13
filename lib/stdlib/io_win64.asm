; picoC I/O standard library – Windows x64 (MS ABI)
BITS 64

global println
global println_double
global println_str
global println_bool
global println_char
global readln
global read_int
global read_double

global __println_fmt_int
global __println_fmt_double
global __println_fmt_str
global __println_fmt_char
global __println_bool_true
global __println_bool_false

extern printf
extern exit
extern GetStdHandle
extern ReadFile
extern memcpy
extern heap_alloc

section .data
    __println_fmt_int db "%lld", 10, 0
    __println_fmt_double db "%g", 10, 0
    __println_fmt_str db "%s", 10, 0
    __println_fmt_char db "%c", 10, 0
    __println_bool_true db "true", 0
    __println_bool_false db "false", 0

section .text

; println(value: int64 in rcx) -> void
println:
    push rbp
    mov rbp, rsp
    mov rdx, rcx
    lea rcx, [rel __println_fmt_int]
    sub rsp, 32
    call printf
    add rsp, 32
    pop rbp
    ret

; println_double(value: double bits in rcx) -> void
println_double:
    push rbp
    mov rbp, rsp
    mov [rbp+16], rcx
    movsd xmm1, qword [rbp+16]
    mov rdx, rcx
    lea rcx, [rel __println_fmt_double]
    sub rsp, 32
    call printf
    add rsp, 32
    pop rbp
    ret

; println_str(ptr: char* in rcx) -> void
println_str:
    push rbp
    mov rbp, rsp
    mov rdx, rcx
    lea rcx, [rel __println_fmt_str]
    sub rsp, 32
    call printf
    add rsp, 32
    pop rbp
    ret

; println_bool(value: bool in rcx) -> void
println_bool:
    push rbp
    mov rbp, rsp
    cmp rcx, 0
    je .println_bool_false
    lea rdx, [rel __println_bool_true]
    jmp .println_bool_out
.println_bool_false:
    lea rdx, [rel __println_bool_false]
.println_bool_out:
    lea rcx, [rel __println_fmt_str]
    sub rsp, 32
    call printf
    add rsp, 32
    pop rbp
    ret

; println_char(value: char in rcx) -> void
println_char:
    push rbp
    mov rbp, rsp
    mov rdx, rcx
    lea rcx, [rel __println_fmt_char]
    sub rsp, 32
    call printf
    add rsp, 32
    pop rbp
    ret

; readln() -> string (ptr in rax)
readln:
    push rbp
    mov rbp, rsp
    push rsi
    push rdi
    sub rsp, 320
    mov rcx, -10
    sub rsp, 32
    call GetStdHandle
    add rsp, 32
    mov rcx, rax
    lea rdx, [rbp-296]
    mov r8, 255
    lea r9, [rbp-304]
    sub rsp, 48
    mov qword [rsp+32], 0
    call ReadFile
    add rsp, 48
    mov rax, [rbp-304]
    cmp rax, 0
    jle .Lwreadln_empty
    xor rcx, rcx
.Lwreadln_find_nl:
    cmp rcx, [rbp-304]
    jge .Lwreadln_no_nl
    cmp byte [rbp-296+rcx], 10
    je .Lwreadln_found_nl
    inc rcx
    jmp .Lwreadln_find_nl
.Lwreadln_found_nl:
    test rcx, rcx
    jz .Lwreadln_do_copy
    cmp byte [rbp-296+rcx-1], 13
    jne .Lwreadln_do_copy
    dec rcx
.Lwreadln_do_copy:
    mov byte [rbp-296+rcx], 0
    jmp .Lwreadln_copy
.Lwreadln_no_nl:
    mov rcx, [rbp-304]
    mov byte [rbp-296+rcx], 0
.Lwreadln_copy:
    lea rcx, [rcx + 1]
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov [rbp-312], rax
    mov rdx, rax
    xor r8, r8
.Lwreadln_cpy:
    cmp byte [rbp-296+r8], 0
    je .Lwreadln_done
    mov al, [rbp-296+r8]
    mov [rdx], al
    inc r8
    inc rdx
    jmp .Lwreadln_cpy
.Lwreadln_done:
    mov byte [rdx], 0
    mov rax, [rbp-312]
    add rsp, 320
    pop rdi
    pop rsi
    pop rbp
    ret
.Lwreadln_empty:
    mov rcx, 1
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov byte [rax], 0
    add rsp, 320
    pop rdi
    pop rsi
    pop rbp
    ret

; read_int() -> int64 in rax
read_int:
    push rbp
    mov rbp, rsp
    push rsi
    sub rsp, 344
    mov rcx, -10
    sub rsp, 32
    call GetStdHandle
    add rsp, 32
    mov rcx, rax
    lea rdx, [rbp-288]
    mov r8, 255
    lea r9, [rbp-296]
    sub rsp, 48
    mov qword [rsp+32], 0
    call ReadFile
    add rsp, 48
    mov rax, [rbp-296]
    cmp rax, 0
    jle .wread_int_zero
    mov byte [rbp-288+rax], 0
    lea rsi, [rbp-288]
    xor eax, eax
    xor r8, r8
.wread_int_skip:
    mov cl, [rsi]
    cmp cl, 0
    je .wread_int_zero
    cmp cl, ' '
    je .wread_int_skip_inc
    cmp cl, 9
    je .wread_int_skip_inc
    cmp cl, 10
    je .wread_int_skip_inc
    cmp cl, 13
    je .wread_int_skip_inc
    cmp cl, '-'
    je .wread_int_neg
    cmp cl, '0'
    jb .wread_int_zero
    cmp cl, '9'
    ja .wread_int_zero
    jmp .wread_int_loop
.wread_int_skip_inc:
    inc rsi
    jmp .wread_int_skip
.wread_int_neg:
    inc rsi
    mov r8, 1
    mov cl, [rsi]
    cmp cl, '0'
    jb .wread_int_zero
    cmp cl, '9'
    ja .wread_int_zero
.wread_int_loop:
    mov cl, [rsi]
    cmp cl, 0
    je .wread_int_done
    cmp cl, '0'
    jb .wread_int_done
    cmp cl, '9'
    ja .wread_int_done
    imul rax, 10
    movzx rcx, cl
    add rax, rcx
    sub rax, '0'
    inc rsi
    jmp .wread_int_loop
.wread_int_done:
    cmp r8, 1
    jne .wread_int_ret
    neg rax
.wread_int_ret:
    add rsp, 344
    pop rsi
    pop rbp
    ret
.wread_int_zero:
    xor eax, eax
    add rsp, 344
    pop rsi
    pop rbp
    ret

; read_double() -> double (xmm0, bits in rax)
read_double:
    push rbp
    mov rbp, rsp
    push rsi
    sub rsp, 360
    mov rcx, -10
    sub rsp, 32
    call GetStdHandle
    add rsp, 32
    mov rcx, rax
    lea rdx, [rbp-288]
    mov r8, 255
    lea r9, [rbp-296]
    sub rsp, 48
    mov qword [rsp+32], 0
    call ReadFile
    add rsp, 48
    mov rax, [rbp-296]
    cmp rax, 0
    jle .wreaddbl_zero
    mov byte [rbp-288+rax], 0
    lea rsi, [rbp-288]
    xor r8, r8
    xor r9, r9
    xor r10, r10
    mov r11, 1
.wreaddbl_skip:
    mov al, [rsi]
    cmp al, 0
    je .wreaddbl_zero
    cmp al, ' '
    je .wreaddbl_skip_inc
    cmp al, 9
    je .wreaddbl_skip_inc
    cmp al, 10
    je .wreaddbl_skip_inc
    cmp al, 13
    je .wreaddbl_skip_inc
    cmp al, '-'
    je .wreaddbl_neg
    cmp al, '0'
    jb .wreaddbl_zero
    cmp al, '9'
    ja .wreaddbl_zero
    jmp .wreaddbl_int
.wreaddbl_skip_inc:
    inc rsi
    jmp .wreaddbl_skip
.wreaddbl_neg:
    inc rsi
    mov r10, 1
    mov al, [rsi]
    cmp al, '0'
    jb .wreaddbl_zero
    cmp al, '9'
    ja .wreaddbl_zero
.wreaddbl_int:
    mov al, [rsi]
    cmp al, 0
    je .wreaddbl_conv
    cmp al, ' '
    je .wreaddbl_conv
    cmp al, 9
    je .wreaddbl_conv
    cmp al, 10
    je .wreaddbl_conv
    cmp al, '.'
    je .wreaddbl_frac
    cmp al, '0'
    jb .wreaddbl_conv
    cmp al, '9'
    ja .wreaddbl_conv
    imul r8, 10
    movzx rax, al
    add r8, rax
    sub r8, '0'
    inc rsi
    jmp .wreaddbl_int
.wreaddbl_frac:
    inc rsi
    xor r9, r9
    mov r11, 1
.wreaddbl_frac_loop:
    mov al, [rsi]
    cmp al, 0
    je .wreaddbl_conv
    cmp al, '0'
    jb .wreaddbl_conv
    cmp al, '9'
    ja .wreaddbl_conv
    imul r9, 10
    movzx rcx, al
    add r9, rcx
    sub r9, '0'
    imul r11, 10
    inc rsi
    jmp .wreaddbl_frac_loop
.wreaddbl_conv:
    cvtsi2sd xmm0, r8
    cvtsi2sd xmm1, r9
    cvtsi2sd xmm2, r11
    divsd xmm1, xmm2
    addsd xmm0, xmm1
    cmp r10, 1
    jne .wreaddbl_ret
    mov rax, 0x8000000000000000
    mov [rbp-8], rax
    xorpd xmm0, [rbp-8]
.wreaddbl_ret:
    movq rax, xmm0
    add rsp, 360
    pop rsi
    pop rbp
    ret
.wreaddbl_zero:
    xorpd xmm0, xmm0
    movq rax, xmm0
    add rsp, 360
    pop rsi
    pop rbp
    ret

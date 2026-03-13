; picoC I/O standard library – Linux x64 (SysV ABI)
BITS 64
default rel

global println
global println_double
global println_str
global println_bool
global println_char
global readln
global read_int
global read_double

global __println_bool_true
global __println_bool_false
global __println_newline
global __println_minus
global __println_dot

extern memcpy
extern heap_alloc

section .data
    __println_bool_true db "true", 10, 0
    __println_bool_false db "false", 10, 0
    __println_newline db 10, 0
    __println_minus db "-", 0
    __println_dot db ".", 0

section .bss
    stdin_buf: resb 256
    stdin_buf_len: resq 1
    __read_line_buf: resb 256

section .text

; println(value: int64 in rdi) -> void
println:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    lea rsi, [rsp]
    mov r8, rsi
    add rsi, 20
    mov byte [rsi], 10
    mov rax, rdi
    cmp rax, 0
    jne .Lconvert
    dec rsi
    mov byte [rsi], '0'
    jmp .Lwrite
.Lconvert:
    cmp rax, 0
    jge .Lpos
    push rax
    lea rsi, [rel __println_minus]
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    pop rax
    neg rax
    lea rsi, [r8+20]
.Lpos:
    xor rdx, rdx
.Lloop:
    xor rdx, rdx
    mov rcx, 10
    div rcx
    dec rsi
    add dl, '0'
    mov [rsi], dl
    test rax, rax
    jnz .Lloop
.Lwrite:
    lea rdx, [r8+21]
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
    mov rsp, rbp
    pop rbp
    ret

; println_bool(value: bool in rdi) -> void
println_bool:
    push rbp
    mov rbp, rsp
    cmp rdi, 0
    je .Lbool_false
    lea rsi, [rel __println_bool_true]
    mov rdx, 5
    jmp .Lbool_write
.Lbool_false:
    lea rsi, [rel __println_bool_false]
    mov rdx, 6
.Lbool_write:
    mov rax, 1
    mov rdi, 1
    syscall
    pop rbp
    ret

; println_str(ptr: char* in rdi) -> void
println_str:
    push rbp
    mov rbp, rsp
    mov rsi, rdi
    xor rdx, rdx
.Lstr_len:
    cmp byte [rsi + rdx], 0
    je .Lstr_write
    inc rdx
    jmp .Lstr_len
.Lstr_write:
    mov rax, 1
    mov rdi, 1
    syscall
    lea rsi, [rel __println_newline]
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    pop rbp
    ret

; println_char(value: char in rdi) -> void
println_char:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov byte [rbp-2], dil
    mov byte [rbp-1], 10
    lea rsi, [rbp-2]
    mov rdx, 2
    mov rax, 1
    mov rdi, 1
    syscall
    add rsp, 16
    pop rbp
    ret

; println_double(value: double bits in rdi) -> void
println_double:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov [rbp-8], rdi
    movsd xmm0, qword [rbp-8]
    xorpd xmm1, xmm1
    ucomisd xmm0, xmm1
    jae .Ldbl_positive
    push rdi
    lea rsi, [rel __println_minus]
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    pop rdi
    movsd xmm0, qword [rbp-8]
    mov rax, 0x8000000000000000
    mov [rbp-16], rax
    xorpd xmm0, [rbp-16]
.Ldbl_positive:
    cvttsd2si rax, xmm0
    mov [rbp-24], rax
    lea rsi, [rbp-48]
    add rsi, 20
    mov byte [rsi], 0
    mov rax, [rbp-24]
    test rax, rax
    jnz .Ldbl_iconv
    dec rsi
    mov byte [rsi], '0'
    jmp .Ldbl_iwrite
.Ldbl_iconv:
.Ldbl_iloop:
    xor rdx, rdx
    mov rcx, 10
    div rcx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    test rax, rax
    jnz .Ldbl_iloop
.Ldbl_iwrite:
    lea rdx, [rbp-48]
    add rdx, 20
    sub rdx, rsi
    mov rax, 1
    mov rdi, 1
    syscall
    cvtsi2sd xmm1, qword [rbp-24]
    movsd xmm0, qword [rbp-8]
    mov rax, 0x7FFFFFFFFFFFFFFF
    mov [rbp-16], rax
    andpd xmm0, [rbp-16]
    subsd xmm0, xmm1
    mov rax, 1000000
    cvtsi2sd xmm1, rax
    mulsd xmm0, xmm1
    roundsd xmm0, xmm0, 0
    cvttsd2si rax, xmm0
    mov [rbp-32], rax
    test rax, rax
    jz .Ldbl_newline
    lea rsi, [rel __println_dot]
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    lea rsi, [rbp-48]
    add rsi, 20
    mov byte [rsi], 0
    mov rax, [rbp-32]
    mov rcx, 6
.Ldbl_floop:
    xor rdx, rdx
    push rcx
    mov rcx, 10
    div rcx
    pop rcx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    dec rcx
    jnz .Ldbl_floop
    lea r9, [rsi+5]
.Ldbl_strip:
    cmp r9, rsi
    jl .Ldbl_stripped
    cmp byte [r9], '0'
    jne .Ldbl_stripped
    dec r9
    jmp .Ldbl_strip
.Ldbl_stripped:
    mov rdx, r9
    sub rdx, rsi
    inc rdx
    mov rax, 1
    mov rdi, 1
    syscall
.Ldbl_newline:
    lea rsi, [rel __println_newline]
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    add rsp, 64
    pop rbp
    ret

; readln() -> string (ptr in rax)
readln:
    push rbp
    mov rbp, rsp
    sub rsp, 272
    mov rcx, [rel stdin_buf_len]
    cmp rcx, 0
    jg .Lreadln_has_buf
.Lreadln_read:
    xor edi, edi
    lea rsi, [rsp]
    mov edx, 255
    xor eax, eax
    syscall
    cmp rax, 0
    jle .Lreadln_empty
    xor rcx, rcx
.Lreadln_find_nl:
    cmp rcx, rax
    jge .Lreadln_no_nl
    cmp byte [rsp + rcx], 10
    je .Lreadln_found_nl
    inc rcx
    jmp .Lreadln_find_nl
.Lreadln_found_nl:
    test rcx, rcx
    jz .Lreadln_do_copy
    cmp byte [rsp + rcx - 1], 13
    jne .Lreadln_do_copy
    dec rcx
.Lreadln_do_copy:
    mov byte [rsp + rcx], 0
    lea r8, [rax]
    sub r8, rcx
    dec r8
    jle .Lreadln_copy
    lea rdi, [rel stdin_buf]
    lea rsi, [rsp + rcx + 1]
    mov rdx, r8
    mov [rel stdin_buf_len], r8
.Lreadln_save_loop:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    dec rdx
    jnz .Lreadln_save_loop
    jmp .Lreadln_copy
.Lreadln_no_nl:
    mov byte [rsp + rax], 0
    mov rcx, rax
    xor r8, r8
    mov [rel stdin_buf_len], r8
.Lreadln_copy:
    mov [rbp-16], rcx
    lea rdi, [rcx + 1]
    call heap_alloc
    mov [rbp-8], rax
    mov rdx, rax
    mov rcx, [rbp-16]
    xor r8, r8
.Lreadln_cpy:
    cmp r8, rcx
    jge .Lreadln_done
    mov al, [rsp + r8]
    mov [rdx], al
    inc r8
    inc rdx
    jmp .Lreadln_cpy
.Lreadln_done:
    mov byte [rdx], 0
    mov rax, [rbp-8]
    add rsp, 272
    pop rbp
    ret
.Lreadln_has_buf:
    lea rsi, [rel stdin_buf]
    xor rcx, rcx
.Lreadln_buf_find:
    cmp rcx, [rel stdin_buf_len]
    jge .Lreadln_buf_no_nl
    cmp byte [rsi + rcx], 10
    je .Lreadln_buf_found
    inc rcx
    jmp .Lreadln_buf_find
.Lreadln_buf_found:
    mov [rbp-16], rcx
    lea rdi, [rcx + 1]
    call heap_alloc
    mov [rbp-8], rax
    mov rdx, rax
    mov rcx, [rbp-16]
    xor r8, r8
.Lreadln_buf_cpy:
    cmp r8, rcx
    jge .Lreadln_buf_cpy_done
    mov al, [rsi + r8]
    mov [rdx], al
    inc r8
    inc rdx
    jmp .Lreadln_buf_cpy
.Lreadln_buf_cpy_done:
    mov byte [rdx], 0
    mov r8, [rel stdin_buf_len]
    mov rcx, [rbp-16]
    sub r8, rcx
    dec r8
    mov [rel stdin_buf_len], r8
    jle .Lreadln_buf_ret
    lea rdi, [rel stdin_buf]
    lea rsi, [rel stdin_buf]
    add rsi, [rbp-16]
    inc rsi
    mov rdx, r8
.Lreadln_buf_shift:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    dec rdx
    jnz .Lreadln_buf_shift
.Lreadln_buf_ret:
    mov rax, [rbp-8]
    add rsp, 272
    pop rbp
    ret
.Lreadln_buf_no_nl:
    mov rcx, [rel stdin_buf_len]
    xor r8, r8
    mov [rel stdin_buf_len], r8
    mov [rbp-16], rcx
    lea rdi, [rcx + 1]
    call heap_alloc
    mov [rbp-8], rax
    mov rdx, rax
    mov rcx, [rbp-16]
    xor r8, r8
.Lreadln_buf_nl_cpy:
    cmp r8, rcx
    jge .Lreadln_buf_nl_done
    mov al, [rsi + r8]
    mov [rdx], al
    inc r8
    inc rdx
    jmp .Lreadln_buf_nl_cpy
.Lreadln_buf_nl_done:
    mov byte [rdx], 0
    mov rax, [rbp-8]
    add rsp, 272
    pop rbp
    ret
.Lreadln_empty:
    mov edi, 1
    call heap_alloc
    mov byte [rax], 0
    add rsp, 272
    pop rbp
    ret

; __read_line helper: read next line into __read_line_buf, return ptr in rsi
__read_line:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    lea rdi, [rel __read_line_buf]
    mov rcx, [rel stdin_buf_len]
    cmp rcx, 0
    jg __read_line_has_buf
__read_line_read:
    xor eax, eax
    xor edi, edi
    lea rsi, [rel __read_line_buf]
    mov edx, 255
    syscall
    cmp rax, 0
    jle __read_line_empty
    xor rcx, rcx
__read_line_find:
    cmp rcx, rax
    jge __read_line_no_nl
    cmp byte [rsi + rcx], 10
    je __read_line_found
    inc rcx
    jmp __read_line_find
__read_line_found:
    mov byte [rsi + rcx], 0
    lea r8, [rax]
    sub r8, rcx
    dec r8
    jle __read_line_done
    mov [rel stdin_buf_len], r8
    lea rdi, [rel stdin_buf]
    lea rsi, [rel __read_line_buf]
    add rsi, rcx
    inc rsi
    mov rdx, r8
__read_line_save:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    dec rdx
    jnz __read_line_save
    jmp __read_line_done
__read_line_no_nl:
    mov byte [rsi + rax], 0
    xor r8, r8
    mov [rel stdin_buf_len], r8
__read_line_done:
    lea rsi, [rel __read_line_buf]
    add rsp, 32
    pop rbp
    ret
__read_line_has_buf:
    lea rsi, [rel stdin_buf]
    xor rcx, rcx
__read_line_buf_find:
    cmp rcx, [rel stdin_buf_len]
    jge __read_line_buf_all
    cmp byte [rsi + rcx], 10
    je __read_line_buf_found
    inc rcx
    jmp __read_line_buf_find
__read_line_buf_found:
    mov [rbp-8], rcx
    lea rdi, [rel __read_line_buf]
    xor r8, r8
__read_line_buf_cpy_loop:
    cmp r8, rcx
    jge __read_line_buf_cpy_done
    mov al, [rsi + r8]
    mov [rdi + r8], al
    inc r8
    jmp __read_line_buf_cpy_loop
__read_line_buf_cpy_done:
    mov byte [rdi + rcx], 0
    mov rcx, [rbp-8]
    mov r8, [rel stdin_buf_len]
    sub r8, rcx
    dec r8
    mov [rel stdin_buf_len], r8
    jle __read_line_buf_ret
    lea rdi, [rel stdin_buf]
    lea rsi, [rel stdin_buf]
    add rsi, [rbp-8]
    inc rsi
    mov rdx, r8
__read_line_buf_shift:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    dec rdx
    jnz __read_line_buf_shift
__read_line_buf_ret:
    lea rsi, [rel __read_line_buf]
    add rsp, 32
    pop rbp
    ret
__read_line_buf_all:
    mov rcx, [rel stdin_buf_len]
    xor r8, r8
    mov [rel stdin_buf_len], r8
    lea rdi, [rel __read_line_buf]
__read_line_buf_all_cpy:
    cmp r8, rcx
    jge __read_line_buf_all_done
    mov al, [rsi + r8]
    mov [rdi + r8], al
    inc r8
    jmp __read_line_buf_all_cpy
__read_line_buf_all_done:
    mov byte [rdi + rcx], 0
    lea rsi, [rel __read_line_buf]
    add rsp, 32
    pop rbp
    ret
__read_line_empty:
    lea rsi, [rel __read_line_buf]
    mov byte [rsi], 0
    add rsp, 32
    pop rbp
    ret

; read_int() -> int64 in rax
read_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    call __read_line
    xor eax, eax
    xor ecx, ecx
    xor r8, r8
.read_int_skip:
    mov cl, [rsi]
    cmp cl, 0
    je .read_int_zero
    cmp cl, ' '
    je .read_int_skip_inc
    cmp cl, 9
    je .read_int_skip_inc
    cmp cl, 10
    je .read_int_skip_inc
    cmp cl, '-'
    je .read_int_neg
    cmp cl, '0'
    jb .read_int_zero
    cmp cl, '9'
    ja .read_int_zero
    jmp .read_int_loop
.read_int_skip_inc:
    inc rsi
    jmp .read_int_skip
.read_int_neg:
    inc rsi
    mov r8, 1
    mov cl, [rsi]
    cmp cl, '0'
    jb .read_int_zero
    cmp cl, '9'
    ja .read_int_zero
.read_int_loop:
    mov cl, [rsi]
    cmp cl, 0
    je .read_int_done
    cmp cl, '0'
    jb .read_int_done
    cmp cl, '9'
    ja .read_int_done
    imul rax, 10
    movzx rcx, cl
    add rax, rcx
    sub rax, '0'
    inc rsi
    jmp .read_int_loop
.read_int_done:
    cmp r8, 1
    jne .read_int_ret
    neg rax
.read_int_ret:
    add rsp, 32
    pop rbp
    ret
.read_int_zero:
    xor eax, eax
    add rsp, 32
    pop rbp
    ret

; read_double() -> double (xmm0, bits in rax)
read_double:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    call __read_line
    xor r8, r8
    xor r9, r9
    xor r10, r10
    mov r11, 1
.readdbl_skip:
    mov al, [rsi]
    cmp al, 0
    je .readdbl_zero
    cmp al, ' '
    je .readdbl_skip_inc
    cmp al, 9
    je .readdbl_skip_inc
    cmp al, 10
    je .readdbl_skip_inc
    cmp al, '-'
    je .readdbl_neg
    cmp al, '0'
    jb .readdbl_zero
    cmp al, '9'
    ja .readdbl_zero
    jmp .readdbl_int
.readdbl_skip_inc:
    inc rsi
    jmp .readdbl_skip
.readdbl_neg:
    inc rsi
    mov r10, 1
    mov al, [rsi]
    cmp al, '0'
    jb .readdbl_zero
    cmp al, '9'
    ja .readdbl_zero
.readdbl_int:
    mov al, [rsi]
    cmp al, 0
    je .readdbl_conv
    cmp al, ' '
    je .readdbl_conv
    cmp al, 9
    je .readdbl_conv
    cmp al, 10
    je .readdbl_conv
    cmp al, '.'
    je .readdbl_frac
    cmp al, '0'
    jb .readdbl_conv
    cmp al, '9'
    ja .readdbl_conv
    imul r8, 10
    movzx rax, al
    add r8, rax
    sub r8, '0'
    inc rsi
    jmp .readdbl_int
.readdbl_frac:
    inc rsi
    xor r9, r9
    mov r11, 1
.readdbl_frac_loop:
    mov al, [rsi]
    cmp al, 0
    je .readdbl_conv
    cmp al, '0'
    jb .readdbl_conv
    cmp al, '9'
    ja .readdbl_conv
    imul r9, 10
    movzx rcx, al
    add r9, rcx
    sub r9, '0'
    imul r11, 10
    inc rsi
    jmp .readdbl_frac_loop
.readdbl_conv:
    cvtsi2sd xmm0, r8
    cvtsi2sd xmm1, r9
    cvtsi2sd xmm2, r11
    divsd xmm1, xmm2
    addsd xmm0, xmm1
    cmp r10, 1
    jne .readdbl_ret
    mov rax, 0x8000000000000000
    mov [rbp-8], rax
    xorpd xmm0, [rbp-8]
.readdbl_ret:
    movq rax, xmm0
    add rsp, 32
    pop rbp
    ret
.readdbl_zero:
    xorpd xmm0, xmm0
    movq rax, xmm0
    add rsp, 32
    pop rbp
    ret

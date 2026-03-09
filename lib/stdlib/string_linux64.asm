; picoC standard library: string (Linux x86_64, SysV ABI)
BITS 64

section .text

global str_concat
global str_from_char
global str_cmp
global str_len
global substr
global substr_step
global char_at
global set_char_at
global str_slice_set

extern heap_alloc
extern memcpy

; ---- str_concat(a: char*, b: char*) -> char* ----
; rdi = a, rsi = b
str_concat:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    xor rcx, rcx
.Lconcat_len1:
    cmp byte [rdi + rcx], 0
    je .Lconcat_len1_done
    inc rcx
    jmp .Lconcat_len1
.Lconcat_len1_done:
    mov rdi, [rbp-16]
    xor rdx, rdx
.Lconcat_len2:
    cmp byte [rdi + rdx], 0
    je .Lconcat_len2_done
    inc rdx
    jmp .Lconcat_len2
.Lconcat_len2_done:
    lea rdi, [rcx + rdx + 1]
    call heap_alloc
    mov [rbp-24], rax
    mov rdi, [rbp-8]
    mov rsi, rax
.Lconcat_cpy1:
    mov cl, [rdi]
    mov [rsi], cl
    inc rdi
    inc rsi
    test cl, cl
    jnz .Lconcat_cpy1
    dec rsi
    mov rdi, [rbp-16]
.Lconcat_cpy2:
    mov cl, [rdi]
    mov [rsi], cl
    inc rdi
    inc rsi
    test cl, cl
    jnz .Lconcat_cpy2
    mov rax, [rbp-24]
    add rsp, 32
    pop rbp
    ret

; ---- str_from_char(c: char) -> char* ----
; rdi = c (byte in dil)
str_from_char:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov [rbp-1], dil
    mov edi, 2
    call heap_alloc
    mov cl, [rbp-1]
    mov [rax], cl
    mov byte [rax+1], 0
    add rsp, 16
    pop rbp
    ret

; ---- str_cmp(a: char*, b: char*) -> int ----
; rdi = a, rsi = b
str_cmp:
.Lstrcmp_loop:
    movzx eax, byte [rdi]
    movzx ecx, byte [rsi]
    sub eax, ecx
    jnz .Lstrcmp_done
    cmp byte [rdi], 0
    je .Lstrcmp_done
    inc rdi
    inc rsi
    jmp .Lstrcmp_loop
.Lstrcmp_done:
    movsx rax, eax
    ret

; ---- str_len(s: char*) -> int ----
; rdi = s
str_len:
    xor eax, eax
.Lstrlen_loop:
    cmp byte [rdi + rax], 0
    je .Lstrlen_done
    inc rax
    jmp .Lstrlen_loop
.Lstrlen_done:
    ret

; ---- substr(s, start, length) -> string ----
; rdi=s, rsi=start, rdx=length
substr:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    mov [rbp-24], rdx
    test rdi, rdi
    jz .substr_empty
    cmp rsi, 0
    jl .substr_empty
    cmp rdx, 0
    jle .substr_empty
    mov rdi, [rbp-8]
    xor rcx, rcx
.substr_len:
    cmp byte [rdi+rcx], 0
    je .substr_len_done
    inc rcx
    jmp .substr_len
.substr_len_done:
    cmp [rbp-16], rcx
    jge .substr_empty
    mov r8, rcx
    sub r8, [rbp-16]
    mov r9, [rbp-24]
    cmp r9, r8
    cmova r9, r8
    lea rdi, [r9+1]
    call heap_alloc
    mov r10, rax
    mov [rbp-32], r10
    mov [rbp-40], r9
    mov rdi, r10
    mov rsi, [rbp-8]
    add rsi, [rbp-16]
    mov rdx, r9
    call memcpy
    mov r10, [rbp-32]
    mov r9, [rbp-40]
    mov byte [r10+r9], 0
    mov rax, r10
    jmp .substr_done
.substr_empty:
    mov edi, 1
    call heap_alloc
    mov byte [rax], 0
.substr_done:
    add rsp, 48
    pop rbp
    ret

; ---- substr_step(s, start, end, step) -> string ----
; rdi=s, rsi=start, rdx=end, rcx=step
substr_step:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    mov [rbp-24], rdx
    mov [rbp-32], rcx
    test rdi, rdi
    jz .substep_empty
    cmp rcx, 0
    je .substep_empty
    xor rax, rax
.substep_len:
    cmp byte [rdi+rax], 0
    je .substep_len_done
    inc rax
    jmp .substep_len
.substep_len_done:
    mov [rbp-40], rax
    mov rsi, [rbp-16]
    xor r8, r8
    cmp rsi, 0
    cmovl rsi, r8
    cmp rsi, rax
    cmovg rsi, rax
    mov [rbp-16], rsi
    mov rdx, [rbp-24]
    cmp rdx, rax
    cmovg rdx, rax
    mov [rbp-24], rdx
    xor r9, r9
    mov r10, rsi
    mov rcx, [rbp-32]
    cmp rcx, 0
    jg .substep_count_pos
.substep_count_neg:
    cmp r10, rdx
    jle .substep_count_done
    cmp r10, rax
    jge .substep_count_done
    cmp r10, 0
    jl .substep_count_done
    inc r9
    add r10, rcx
    jmp .substep_count_neg
.substep_count_pos:
    cmp r10, rdx
    jge .substep_count_done
    cmp r10, rax
    jge .substep_count_done
    cmp r10, 0
    jl .substep_count_done
    inc r9
    add r10, rcx
    jmp .substep_count_pos
.substep_count_done:
    test r9, r9
    jz .substep_empty
    mov [rbp-48], r9
    lea rdi, [r9+1]
    call heap_alloc
    mov [rbp-56], rax
    mov r10, [rbp-16]
    mov rcx, [rbp-32]
    mov rdi, [rbp-8]
    mov rdx, rax
    mov r9, [rbp-48]
    xor r8, r8
.substep_copy:
    cmp r8, r9
    jge .substep_copy_done
    movzx eax, byte [rdi+r10]
    mov [rdx+r8], al
    inc r8
    add r10, rcx
    jmp .substep_copy
.substep_copy_done:
    mov byte [rdx+r8], 0
    mov rax, [rbp-56]
    jmp .substep_done
.substep_empty:
    mov edi, 1
    call heap_alloc
    mov byte [rax], 0
.substep_done:
    add rsp, 64
    pop rbp
    ret

; ---- char_at(s, index) -> char ----
; rdi=s, rsi=index
char_at:
    cmp rsi, 0
    jl .char_at_bad
    xor rcx, rcx
.char_at_loop:
    mov al, [rdi + rcx]
    cmp al, 0
    je .char_at_bad
    cmp rcx, rsi
    je .char_at_found
    inc rcx
    jmp .char_at_loop
.char_at_found:
    movzx eax, al
    ret
.char_at_bad:
    xor eax, eax
    ret

; ---- set_char_at(s, index, c) -> void ----
; rdi=s, rsi=index, rdx=c
set_char_at:
    cmp rsi, 0
    jl .set_char_at_done
    xor rcx, rcx
.set_char_at_len:
    cmp byte [rdi + rcx], 0
    je .set_char_at_check
    inc rcx
    jmp .set_char_at_len
.set_char_at_check:
    cmp rsi, rcx
    jge .set_char_at_done
    mov [rdi + rsi], dl
.set_char_at_done:
    ret

; ---- str_slice_set(s, start, end, step, replacement) -> void ----
; rdi=s, rsi=start, rdx=end, rcx=step, r8=replacement
str_slice_set:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    mov [rbp-24], rdx
    mov [rbp-32], rcx
    mov [rbp-40], r8
    test rcx, rcx
    jz .Lsls_done
    xor rax, rax
.Lsls_slen:
    cmp byte [rdi + rax], 0
    je .Lsls_slen_done
    inc rax
    jmp .Lsls_slen
.Lsls_slen_done:
    mov [rbp-48], rax
    mov r8, [rbp-40]
    xor r9, r9
.Lsls_rlen:
    cmp byte [r8 + r9], 0
    je .Lsls_rlen_done
    inc r9
    jmp .Lsls_rlen
.Lsls_rlen_done:
    test r9, r9
    jz .Lsls_done
    mov r10, [rbp-16]
    xor r11, r11
    mov rcx, [rbp-32]
    mov rdi, [rbp-8]
    mov rdx, [rbp-24]
    cmp rcx, 0
    jg .Lsls_loop_pos
.Lsls_loop_neg:
    cmp r10, rdx
    jle .Lsls_done
    cmp r10, [rbp-48]
    jge .Lsls_done
    cmp r10, 0
    jl .Lsls_done
    cmp r11, r9
    jge .Lsls_done
    movzx eax, byte [r8 + r11]
    mov [rdi + r10], al
    inc r11
    add r10, rcx
    jmp .Lsls_loop_neg
.Lsls_loop_pos:
    cmp r10, rdx
    jge .Lsls_done
    cmp r10, [rbp-48]
    jge .Lsls_done
    cmp r10, 0
    jl .Lsls_done
    cmp r11, r9
    jge .Lsls_done
    movzx eax, byte [r8 + r11]
    mov [rdi + r10], al
    inc r11
    add r10, rcx
    jmp .Lsls_loop_pos
.Lsls_done:
    add rsp, 48
    pop rbp
    ret

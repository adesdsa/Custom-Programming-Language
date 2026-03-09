; picoC standard library: string (Windows x86_64, MS ABI)
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
str_concat:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    mov [rbp+16], rcx
    mov [rbp+24], rdx
    xor r8, r8
.Lwconcat_len1:
    cmp byte [rcx + r8], 0
    je .Lwconcat_len1_done
    inc r8
    jmp .Lwconcat_len1
.Lwconcat_len1_done:
    mov rcx, [rbp+24]
    xor r9, r9
.Lwconcat_len2:
    cmp byte [rcx + r9], 0
    je .Lwconcat_len2_done
    inc r9
    jmp .Lwconcat_len2
.Lwconcat_len2_done:
    lea rcx, [r8 + r9 + 1]
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov [rbp-8], rax
    mov rcx, [rbp+16]
    mov rdx, rax
.Lwconcat_cpy1:
    mov r8b, [rcx]
    mov [rdx], r8b
    inc rcx
    inc rdx
    test r8b, r8b
    jnz .Lwconcat_cpy1
    dec rdx
    mov rcx, [rbp+24]
.Lwconcat_cpy2:
    mov r8b, [rcx]
    mov [rdx], r8b
    inc rcx
    inc rdx
    test r8b, r8b
    jnz .Lwconcat_cpy2
    mov rax, [rbp-8]
    add rsp, 48
    pop rbp
    ret

; ---- str_from_char(c: char) -> char* ----
str_from_char:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov [rbp-1], cl
    mov rcx, 2
    call heap_alloc
    mov cl, [rbp-1]
    mov [rax], cl
    mov byte [rax+1], 0
    add rsp, 32
    pop rbp
    ret

; ---- str_cmp(a: char*, b: char*) -> int ----
str_cmp:
.Lwstrcmp_loop:
    movzx eax, byte [rcx]
    movzx r8d, byte [rdx]
    sub eax, r8d
    jnz .Lwstrcmp_done
    cmp byte [rcx], 0
    je .Lwstrcmp_done
    inc rcx
    inc rdx
    jmp .Lwstrcmp_loop
.Lwstrcmp_done:
    movsx rax, eax
    ret

; ---- str_len(s: char*) -> int ----
str_len:
    xor eax, eax
.Lwstrlen_loop:
    cmp byte [rcx + rax], 0
    je .Lwstrlen_done
    inc rax
    jmp .Lwstrlen_loop
.Lwstrlen_done:
    ret

; ---- substr(s: string, start: int, length: int) -> string ----
substr:
    push rbp
    mov rbp, rsp
    sub rsp, 80
    mov [rbp-8], rcx
    mov [rbp-16], rdx
    mov [rbp-24], r8
    test rcx, rcx
    jz .substr_empty
    cmp rdx, 0
    jl .substr_empty
    cmp r8, 0
    jle .substr_empty
    mov rcx, [rbp-8]
    xor r9, r9
.substr_len:
    cmp byte [rcx+r9], 0
    je .substr_len_done
    inc r9
    jmp .substr_len
.substr_len_done:
    cmp [rbp-16], r9
    jge .substr_empty
    mov r10, r9
    sub r10, [rbp-16]
    mov r11, [rbp-24]
    cmp r11, r10
    cmova r11, r10
    lea rcx, [r11+1]
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov r10, rax
    mov rcx, r10
    mov rdx, [rbp-8]
    add rdx, [rbp-16]
    mov r8, r11
    sub rsp, 32
    call memcpy
    add rsp, 32
    mov byte [r10+r11], 0
    mov rax, r10
    jmp .substr_done
.substr_empty:
    mov rcx, 1
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov byte [rax], 0
.substr_done:
    add rsp, 80
    pop rbp
    ret

; ---- substr_step(s, start, end, step) -> string ----
substr_step:
    push rbp
    mov rbp, rsp
    sub rsp, 96
    mov [rbp-8], rcx
    mov [rbp-16], rdx
    mov [rbp-24], r8
    mov [rbp-32], r9
    test rcx, rcx
    jz .substep_empty
    cmp r9, 0
    je .substep_empty
    xor rax, rax
.substep_len:
    cmp byte [rcx+rax], 0
    je .substep_len_done
    inc rax
    jmp .substep_len
.substep_len_done:
    mov [rbp-40], rax
    mov rdx, [rbp-16]
    xor r10, r10
    cmp rdx, 0
    cmovl rdx, r10
    cmp rdx, rax
    cmovg rdx, rax
    mov [rbp-16], rdx
    mov r8, [rbp-24]
    cmp r8, rax
    cmovg r8, rax
    mov [rbp-24], r8
    xor r11, r11
    mov r10, rdx
    mov r9, [rbp-32]
    cmp r9, 0
    jg .substep_count_pos
.substep_count_neg:
    cmp r10, r8
    jle .substep_count_done
    cmp r10, rax
    jge .substep_count_done
    cmp r10, 0
    jl .substep_count_done
    inc r11
    add r10, r9
    jmp .substep_count_neg
.substep_count_pos:
    cmp r10, r8
    jge .substep_count_done
    cmp r10, rax
    jge .substep_count_done
    cmp r10, 0
    jl .substep_count_done
    inc r11
    add r10, r9
    jmp .substep_count_pos
.substep_count_done:
    test r11, r11
    jz .substep_empty
    mov [rbp-48], r11
    lea rcx, [r11+1]
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov [rbp-56], rax
    mov r10, [rbp-16]
    mov r9, [rbp-32]
    mov rcx, [rbp-8]
    mov rdx, rax
    mov r11, [rbp-48]
    xor r8, r8
.substep_copy:
    cmp r8, r11
    jge .substep_copy_done
    movzx eax, byte [rcx+r10]
    mov [rdx+r8], al
    inc r8
    add r10, r9
    jmp .substep_copy
.substep_copy_done:
    mov byte [rdx+r8], 0
    mov rax, [rbp-56]
    jmp .substep_done
.substep_empty:
    mov rcx, 1
    sub rsp, 32
    call heap_alloc
    add rsp, 32
    mov byte [rax], 0
.substep_done:
    add rsp, 96
    pop rbp
    ret

; ---- char_at(s: string, index: int) -> char ----
char_at:
    cmp rdx, 0
    jl .char_at_bad
    xor r8, r8
.char_at_loop:
    mov al, [rcx + r8]
    cmp al, 0
    je .char_at_bad
    cmp r8, rdx
    je .char_at_found
    inc r8
    jmp .char_at_loop
.char_at_found:
    movzx eax, al
    ret
.char_at_bad:
    xor eax, eax
    ret

; ---- set_char_at(s: char*, index: int, c: char) -> void ----
set_char_at:
    cmp rdx, 0
    jl .set_char_at_done
    xor r9, r9
.set_char_at_len:
    cmp byte [rcx + r9], 0
    je .set_char_at_check
    inc r9
    jmp .set_char_at_len
.set_char_at_check:
    cmp rdx, r9
    jge .set_char_at_done
    mov [rcx + rdx], r8b
.set_char_at_done:
    ret

; ---- str_slice_set(s, start, end, step, replacement) -> void ----
; 5th arg (replacement) passed on stack at [rbp+48]
str_slice_set:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov [rbp-8], rcx
    mov [rbp-16], rdx
    mov [rbp-24], r8
    mov [rbp-32], r9
    mov rax, [rbp+48]
    mov [rbp-40], rax
    test r9, r9
    jz .Lwsls_done
    xor rax, rax
.Lwsls_slen:
    cmp byte [rcx + rax], 0
    je .Lwsls_slen_done
    inc rax
    jmp .Lwsls_slen
.Lwsls_slen_done:
    mov [rbp-48], rax
    mov r8, [rbp-40]
    xor r9, r9
.Lwsls_rlen:
    cmp byte [r8 + r9], 0
    je .Lwsls_rlen_done
    inc r9
    jmp .Lwsls_rlen
.Lwsls_rlen_done:
    test r9, r9
    jz .Lwsls_done
    mov r10, [rbp-16]
    xor r11, r11
    mov rcx, [rbp-32]
    mov rdi, [rbp-8]
    mov rdx, [rbp-24]
    cmp rcx, 0
    jg .Lwsls_loop_pos
.Lwsls_loop_neg:
    cmp r10, rdx
    jle .Lwsls_done
    cmp r10, [rbp-48]
    jge .Lwsls_done
    cmp r10, 0
    jl .Lwsls_done
    cmp r11, r9
    jge .Lwsls_done
    movzx eax, byte [r8 + r11]
    mov [rdi + r10], al
    inc r11
    add r10, rcx
    jmp .Lwsls_loop_neg
.Lwsls_loop_pos:
    cmp r10, rdx
    jge .Lwsls_done
    cmp r10, [rbp-48]
    jge .Lwsls_done
    cmp r10, 0
    jl .Lwsls_done
    cmp r11, r9
    jge .Lwsls_done
    movzx eax, byte [r8 + r11]
    mov [rdi + r10], al
    inc r11
    add r10, rcx
    jmp .Lwsls_loop_pos
.Lwsls_done:
    add rsp, 64
    pop rbp
    ret

; picoC standard library: string (Linux x86, cdecl)
BITS 32

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
    push ebp
    mov ebp, esp
    sub esp, 24
    mov eax, [ebp+8]
    mov [ebp-4], eax
    mov eax, [ebp+12]
    mov [ebp-8], eax
    xor ecx, ecx
.Lconcat_len1:
    mov eax, [ebp-4]
    cmp byte [eax + ecx], 0
    je .Lconcat_len1_done
    inc ecx
    jmp .Lconcat_len1
.Lconcat_len1_done:
    xor edx, edx
.Lconcat_len2:
    mov eax, [ebp-8]
    cmp byte [eax + edx], 0
    je .Lconcat_len2_done
    inc edx
    jmp .Lconcat_len2
.Lconcat_len2_done:
    lea eax, [ecx + edx + 1]
    push eax
    call heap_alloc
    add esp, 4
    mov [ebp-12], eax
    mov ecx, [ebp-4]
    mov edx, eax
.Lconcat_cpy1:
    mov al, [ecx]
    mov [edx], al
    inc ecx
    inc edx
    test al, al
    jnz .Lconcat_cpy1
    dec edx
    mov ecx, [ebp-8]
.Lconcat_cpy2:
    mov al, [ecx]
    mov [edx], al
    inc ecx
    inc edx
    test al, al
    jnz .Lconcat_cpy2
    mov eax, [ebp-12]
    add esp, 24
    pop ebp
    ret

; ---- str_from_char(c: char) -> char* ----
str_from_char:
    push ebp
    mov ebp, esp
    sub esp, 8
    push 2
    call heap_alloc
    add esp, 4
    mov cl, [ebp+8]
    mov [eax], cl
    mov byte [eax+1], 0
    add esp, 8
    pop ebp
    ret

; ---- str_cmp(a: char*, b: char*) -> int ----
str_cmp:
    mov ecx, [esp+4]
    mov edx, [esp+8]
.Lstrcmp_loop:
    movzx eax, byte [ecx]
    movzx ebx, byte [edx]
    sub eax, ebx
    jnz .Lstrcmp_done
    cmp byte [ecx], 0
    je .Lstrcmp_done
    inc ecx
    inc edx
    jmp .Lstrcmp_loop
.Lstrcmp_done:
    ret

; ---- str_len(s: char*) -> int ----
str_len:
    mov ecx, [esp+4]
    xor eax, eax
.Lstrlen_loop:
    cmp byte [ecx + eax], 0
    je .Lstrlen_done
    inc eax
    jmp .Lstrlen_loop
.Lstrlen_done:
    ret

; ---- substr(s: string, start: int, length: int) -> string ----
substr:
    push ebp
    mov ebp, esp
    push ebx
    sub esp, 28
    mov ecx, [ebp+8]
    mov [ebp-4], ecx
    mov edx, [ebp+12]
    mov [ebp-8], edx
    mov eax, [ebp+16]
    mov [ebp-12], eax
    test ecx, ecx
    jz .substr_empty
    cmp edx, 0
    jl .substr_empty
    cmp eax, 0
    jle .substr_empty
    mov ecx, [ebp-4]
    xor eax, eax
.substr_len:
    cmp byte [ecx+eax], 0
    je .substr_len_done
    inc eax
    jmp .substr_len
.substr_len_done:
    cmp [ebp-8], eax
    jge .substr_empty
    mov ebx, eax
    sub ebx, [ebp-8]
    mov ecx, [ebp-12]
    cmp ecx, ebx
    cmova ecx, ebx
    mov [ebp-16], ecx
    inc ecx
    push ecx
    call heap_alloc
    add esp, 4
    mov [ebp-20], eax
    push dword [ebp-16]
    mov eax, [ebp-4]
    add eax, [ebp-8]
    push eax
    push dword [ebp-20]
    call memcpy
    add esp, 12
    mov eax, [ebp-20]
    add eax, [ebp-16]
    mov byte [eax], 0
    mov eax, [ebp-20]
    jmp .substr_done
.substr_empty:
    push 1
    call heap_alloc
    add esp, 4
    mov byte [eax], 0
.substr_done:
    add esp, 28
    pop ebx
    pop ebp
    ret

; ---- substr_step(s, start, end, step) -> string ----
; cdecl: [ebp+8]=s, [ebp+12]=start, [ebp+16]=end, [ebp+20]=step
substr_step:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    sub esp, 32
    mov ecx, [ebp+8]
    mov edx, [ebp+20]
    test ecx, ecx
    jz .substep_empty
    cmp edx, 0
    je .substep_empty
    xor eax, eax
.substep_len:
    cmp byte [ecx+eax], 0
    je .substep_len_done
    inc eax
    jmp .substep_len
.substep_len_done:
    mov [ebp-4], eax
    mov esi, [ebp+12]
    cmp esi, 0
    jge .substep_s_ok
    xor esi, esi
.substep_s_ok:
    cmp esi, eax
    jle .substep_s_ok2
    mov esi, eax
.substep_s_ok2:
    mov [ebp+12], esi
    mov edi, [ebp+16]
    cmp edi, eax
    jle .substep_e_ok2
    mov edi, eax
.substep_e_ok2:
    mov [ebp+16], edi
    xor ebx, ebx
    mov esi, [ebp+12]
    mov edx, [ebp+20]
    cmp edx, 0
    jg .substep_count_pos
.substep_count_neg:
    cmp esi, edi
    jle .substep_count_done
    cmp esi, eax
    jge .substep_count_done
    cmp esi, 0
    jl .substep_count_done
    inc ebx
    add esi, edx
    jmp .substep_count_neg
.substep_count_pos:
    cmp esi, edi
    jge .substep_count_done
    cmp esi, eax
    jge .substep_count_done
    cmp esi, 0
    jl .substep_count_done
    inc ebx
    add esi, edx
    jmp .substep_count_pos
.substep_count_done:
    test ebx, ebx
    jz .substep_empty
    mov [ebp-8], ebx
    lea eax, [ebx+1]
    push eax
    call heap_alloc
    add esp, 4
    mov [ebp-12], eax
    mov esi, [ebp+12]
    mov edx, [ebp+20]
    mov ecx, [ebp+8]
    mov edi, eax
    xor ebx, ebx
.substep_copy:
    cmp ebx, [ebp-8]
    jge .substep_copy_done
    movzx eax, byte [ecx+esi]
    mov [edi+ebx], al
    inc ebx
    add esi, edx
    jmp .substep_copy
.substep_copy_done:
    mov byte [edi+ebx], 0
    mov eax, [ebp-12]
    jmp .substep_done
.substep_empty:
    push 1
    call heap_alloc
    add esp, 4
    mov byte [eax], 0
.substep_done:
    add esp, 32
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; ---- char_at(s: string, index: int) -> char ----
char_at:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    cmp edx, 0
    jl .Lchar_at_bad
    xor eax, eax
.Lchar_at_loop:
    cmp byte [ecx+eax], 0
    je .Lchar_at_bad
    cmp eax, edx
    je .Lchar_at_found
    inc eax
    jmp .Lchar_at_loop
.Lchar_at_found:
    mov al, [ecx+eax]
    movzx eax, al
    ret
.Lchar_at_bad:
    xor eax, eax
    ret

; ---- set_char_at(s: char*, index: int, c: char) -> void ----
; cdecl: [esp+4]=s, [esp+8]=index, [esp+12]=c
set_char_at:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    cmp edx, 0
    jl .Lsca_done
    xor eax, eax
.Lsca_len:
    cmp byte [ecx + eax], 0
    je .Lsca_check
    inc eax
    jmp .Lsca_len
.Lsca_check:
    cmp edx, eax
    jge .Lsca_done
    mov al, [esp+12]
    mov [ecx + edx], al
.Lsca_done:
    ret

; ---- str_slice_set(s, start, end, step, replacement) -> void ----
; cdecl: [ebp+8]=s, [ebp+12]=start, [ebp+16]=end, [ebp+20]=step, [ebp+24]=replacement
str_slice_set:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    sub esp, 16
    mov ecx, [ebp+8]
    mov edx, [ebp+20]
    test edx, edx
    jz .Lsls32_done
    xor eax, eax
.Lsls32_slen:
    cmp byte [ecx + eax], 0
    je .Lsls32_slen_done
    inc eax
    jmp .Lsls32_slen
.Lsls32_slen_done:
    mov [ebp-4], eax
    mov esi, [ebp+24]
    xor ebx, ebx
.Lsls32_rlen:
    cmp byte [esi + ebx], 0
    je .Lsls32_rlen_done
    inc ebx
    jmp .Lsls32_rlen
.Lsls32_rlen_done:
    test ebx, ebx
    jz .Lsls32_done
    mov [ebp-8], ebx
    mov edi, [ebp+12]
    xor ebx, ebx
    mov edx, [ebp+20]
    cmp edx, 0
    jg .Lsls32_loop_pos
.Lsls32_loop_neg:
    cmp edi, [ebp+16]
    jle .Lsls32_done
    cmp edi, [ebp-4]
    jge .Lsls32_done
    cmp edi, 0
    jl .Lsls32_done
    cmp ebx, [ebp-8]
    jge .Lsls32_done
    movzx eax, byte [esi + ebx]
    mov [ecx + edi], al
    inc ebx
    add edi, edx
    jmp .Lsls32_loop_neg
.Lsls32_loop_pos:
    cmp edi, [ebp+16]
    jge .Lsls32_done
    cmp edi, [ebp-4]
    jge .Lsls32_done
    cmp edi, 0
    jl .Lsls32_done
    cmp ebx, [ebp-8]
    jge .Lsls32_done
    movzx eax, byte [esi + ebx]
    mov [ecx + edi], al
    inc ebx
    add edi, edx
    jmp .Lsls32_loop_pos
.Lsls32_done:
    add esp, 16
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

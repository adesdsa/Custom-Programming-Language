; picoC standard library: string (Windows x86, cdecl)
BITS 32

section .text

global _str_concat
global _str_from_char
global _str_cmp
global _str_len
global _substr
global _substr_step
global _char_at
global _set_char_at
global _str_slice_set

extern _heap_alloc
extern _memcpy

; ---- _str_concat(a: char*, b: char*) -> char* ----
_str_concat:
    push ebp
    mov ebp, esp
    sub esp, 24
    mov eax, [ebp+8]
    mov [ebp-4], eax
    mov eax, [ebp+12]
    mov [ebp-8], eax
    xor ecx, ecx
._Lconcat_len1:
    mov eax, [ebp-4]
    cmp byte [eax + ecx], 0
    je ._Lconcat_len1_done
    inc ecx
    jmp ._Lconcat_len1
._Lconcat_len1_done:
    xor edx, edx
._Lconcat_len2:
    mov eax, [ebp-8]
    cmp byte [eax + edx], 0
    je ._Lconcat_len2_done
    inc edx
    jmp ._Lconcat_len2
._Lconcat_len2_done:
    lea eax, [ecx + edx + 1]
    push eax
    call _heap_alloc
    add esp, 4
    mov [ebp-12], eax
    mov ecx, [ebp-4]
    mov edx, eax
._Lconcat_cpy1:
    mov al, [ecx]
    mov [edx], al
    inc ecx
    inc edx
    test al, al
    jnz ._Lconcat_cpy1
    dec edx
    mov ecx, [ebp-8]
._Lconcat_cpy2:
    mov al, [ecx]
    mov [edx], al
    inc ecx
    inc edx
    test al, al
    jnz ._Lconcat_cpy2
    mov eax, [ebp-12]
    add esp, 24
    pop ebp
    ret

; ---- _str_from_char(c: char) -> char* ----
_str_from_char:
    push ebp
    mov ebp, esp
    push 2
    call _heap_alloc
    add esp, 4
    mov cl, [ebp+8]
    mov [eax], cl
    mov byte [eax+1], 0
    pop ebp
    ret

; ---- _str_cmp(a: char*, b: char*) -> int ----
_str_cmp:
    mov ecx, [esp+4]
    mov edx, [esp+8]
._Lstrcmp_loop:
    movzx eax, byte [ecx]
    movzx ebx, byte [edx]
    sub eax, ebx
    jnz ._Lstrcmp_done
    cmp byte [ecx], 0
    je ._Lstrcmp_done
    inc ecx
    inc edx
    jmp ._Lstrcmp_loop
._Lstrcmp_done:
    ret

; ---- _str_len(s: char*) -> int ----
_str_len:
    mov ecx, [esp+4]
    xor eax, eax
._Lstrlen_loop:
    cmp byte [ecx + eax], 0
    je ._Lstrlen_done
    inc eax
    jmp ._Lstrlen_loop
._Lstrlen_done:
    ret

; ---- _substr(s: string, start: int, length: int) -> string ----
_substr:
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
    jz ._substr_empty
    cmp edx, 0
    jl ._substr_empty
    cmp eax, 0
    jle ._substr_empty
    mov ecx, [ebp-4]
    xor eax, eax
._substr_len:
    cmp byte [ecx+eax], 0
    je ._substr_len_done
    inc eax
    jmp ._substr_len
._substr_len_done:
    cmp [ebp-8], eax
    jge ._substr_empty
    mov ebx, eax
    sub ebx, [ebp-8]
    mov ecx, [ebp-12]
    cmp ecx, ebx
    cmova ecx, ebx
    mov [ebp-16], ecx
    inc ecx
    push ecx
    call _heap_alloc
    add esp, 4
    mov [ebp-20], eax
    push dword [ebp-16]
    mov eax, [ebp-4]
    add eax, [ebp-8]
    push eax
    push dword [ebp-20]
    call _memcpy
    add esp, 12
    mov eax, [ebp-20]
    add eax, [ebp-16]
    mov byte [eax], 0
    mov eax, [ebp-20]
    jmp ._substr_done
._substr_empty:
    push 1
    call _heap_alloc
    add esp, 4
    mov byte [eax], 0
._substr_done:
    add esp, 28
    pop ebx
    pop ebp
    ret

; ---- _substr_step(s, start, end, step) -> string ----
; cdecl: [ebp+8]=s, [ebp+12]=start, [ebp+16]=end, [ebp+20]=step
_substr_step:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    sub esp, 32
    mov ecx, [ebp+8]
    mov edx, [ebp+20]
    test ecx, ecx
    jz ._substep_empty
    cmp edx, 0
    je ._substep_empty
    xor eax, eax
._substep_len:
    cmp byte [ecx+eax], 0
    je ._substep_len_done
    inc eax
    jmp ._substep_len
._substep_len_done:
    mov [ebp-4], eax
    mov esi, [ebp+12]
    cmp esi, 0
    jge ._substep_s_ok
    xor esi, esi
._substep_s_ok:
    cmp esi, eax
    jle ._substep_s_ok2
    mov esi, eax
._substep_s_ok2:
    mov [ebp+12], esi
    mov edi, [ebp+16]
    cmp edi, eax
    jle ._substep_e_ok2
    mov edi, eax
._substep_e_ok2:
    mov [ebp+16], edi
    xor ebx, ebx
    mov esi, [ebp+12]
    mov edx, [ebp+20]
    cmp edx, 0
    jg ._substep_count_pos
._substep_count_neg:
    cmp esi, edi
    jle ._substep_count_done
    cmp esi, eax
    jge ._substep_count_done
    cmp esi, 0
    jl ._substep_count_done
    inc ebx
    add esi, edx
    jmp ._substep_count_neg
._substep_count_pos:
    cmp esi, edi
    jge ._substep_count_done
    cmp esi, eax
    jge ._substep_count_done
    cmp esi, 0
    jl ._substep_count_done
    inc ebx
    add esi, edx
    jmp ._substep_count_pos
._substep_count_done:
    test ebx, ebx
    jz ._substep_empty
    mov [ebp-8], ebx
    lea eax, [ebx+1]
    push eax
    call _heap_alloc
    add esp, 4
    mov [ebp-12], eax
    mov esi, [ebp+12]
    mov edx, [ebp+20]
    mov ecx, [ebp+8]
    mov edi, eax
    xor ebx, ebx
._substep_copy:
    cmp ebx, [ebp-8]
    jge ._substep_copy_done
    movzx eax, byte [ecx+esi]
    mov [edi+ebx], al
    inc ebx
    add esi, edx
    jmp ._substep_copy
._substep_copy_done:
    mov byte [edi+ebx], 0
    mov eax, [ebp-12]
    jmp ._substep_done
._substep_empty:
    push 1
    call _heap_alloc
    add esp, 4
    mov byte [eax], 0
._substep_done:
    add esp, 32
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; ---- _char_at(s: string, index: int) -> char ----
_char_at:
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

; ---- _set_char_at(s: char*, index: int, c: char) -> void ----
; cdecl: [esp+4]=s, [esp+8]=index, [esp+12]=c
_set_char_at:
    mov ecx, [esp+4]
    mov edx, [esp+8]
    cmp edx, 0
    jl ._Lsca_done
    xor eax, eax
._Lsca_len:
    cmp byte [ecx + eax], 0
    je ._Lsca_check
    inc eax
    jmp ._Lsca_len
._Lsca_check:
    cmp edx, eax
    jge ._Lsca_done
    mov al, [esp+12]
    mov [ecx + edx], al
._Lsca_done:
    ret

; ---- _str_slice_set(s, start, end, step, replacement) -> void ----
; cdecl: [ebp+8]=s, [ebp+12]=start, [ebp+16]=end, [ebp+20]=step, [ebp+24]=replacement
_str_slice_set:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    sub esp, 16
    mov ecx, [ebp+8]
    mov edx, [ebp+20]
    test edx, edx
    jz ._Lsls32_done
    xor eax, eax
._Lsls32_slen:
    cmp byte [ecx + eax], 0
    je ._Lsls32_slen_done
    inc eax
    jmp ._Lsls32_slen
._Lsls32_slen_done:
    mov [ebp-4], eax
    mov esi, [ebp+24]
    xor ebx, ebx
._Lsls32_rlen:
    cmp byte [esi + ebx], 0
    je ._Lsls32_rlen_done
    inc ebx
    jmp ._Lsls32_rlen
._Lsls32_rlen_done:
    test ebx, ebx
    jz ._Lsls32_done
    mov [ebp-8], ebx
    mov edi, [ebp+12]
    xor ebx, ebx
    mov edx, [ebp+20]
    cmp edx, 0
    jg ._Lsls32_loop_pos
._Lsls32_loop_neg:
    cmp edi, [ebp+16]
    jle ._Lsls32_done
    cmp edi, [ebp-4]
    jge ._Lsls32_done
    cmp edi, 0
    jl ._Lsls32_done
    cmp ebx, [ebp-8]
    jge ._Lsls32_done
    movzx eax, byte [esi + ebx]
    mov [ecx + edi], al
    inc ebx
    add edi, edx
    jmp ._Lsls32_loop_neg
._Lsls32_loop_pos:
    cmp edi, [ebp+16]
    jge ._Lsls32_done
    cmp edi, [ebp-4]
    jge ._Lsls32_done
    cmp edi, 0
    jl ._Lsls32_done
    cmp ebx, [ebp-8]
    jge ._Lsls32_done
    movzx eax, byte [esi + ebx]
    mov [ecx + edi], al
    inc ebx
    add edi, edx
    jmp ._Lsls32_loop_pos
._Lsls32_done:
    add esp, 16
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

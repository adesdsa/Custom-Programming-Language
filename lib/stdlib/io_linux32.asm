; picoC I/O standard library - Linux x86 (cdecl)
BITS 32

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

extern heap_alloc

section .data
    __println_bool_true db "true", 10, 0
    __println_bool_false db "false", 10, 0
    __println_newline db 10, 0
    __println_minus db "-", 0
    __println_dot db ".", 0

section .bss
    stdin_buf: resb 256
    stdin_buf_len: resd 1
    __read_line_buf: resb 256

section .text

; println(value: int32 at [ebp+8]) -> void
println:
    push ebp
    mov ebp, esp
    sub esp, 32
    mov edi, esp
    lea esi, [edi+20]
    mov byte [esi], 10
    mov eax, [ebp+8]
    cmp eax, 0
    jne .Lconvert
    dec esi
    mov byte [esi], '0'
    jmp .Lwrite
.Lconvert:
    cmp eax, 0
    jge .Lpos
    push eax
    mov eax, 4
    mov ebx, 1
    mov ecx, __println_minus
    mov edx, 1
    int 0x80
    pop eax
    neg eax
    lea esi, [edi+20]
.Lpos:
    xor edx, edx
.Lloop:
    mov ecx, 10
    div ecx
    dec esi
    add dl, '0'
    mov [esi], dl
    test eax, eax
    jnz .Lloop
.Lwrite:
    lea edx, [edi+21]
    sub edx, esi
    mov ecx, esi
    mov eax, 4
    mov ebx, 1
    int 0x80
    mov esp, ebp
    pop ebp
    ret

; println_bool(value: bool at [ebp+8]) -> void
println_bool:
    push ebp
    mov ebp, esp
    cmp dword [ebp+8], 0
    je .Lbool_false
    mov ecx, __println_bool_true
    mov edx, 5
    jmp .Lbool_write
.Lbool_false:
    mov ecx, __println_bool_false
    mov edx, 6
.Lbool_write:
    mov eax, 4
    mov ebx, 1
    int 0x80
    pop ebp
    ret

; println_str(ptr at [ebp+8]) -> void
println_str:
    push ebp
    mov ebp, esp
    mov ecx, [ebp+8]
    xor edx, edx
.Lstr_len:
    cmp byte [ecx + edx], 0
    je .Lstr_write
    inc edx
    jmp .Lstr_len
.Lstr_write:
    mov eax, 4
    mov ebx, 1
    int 0x80
    mov ecx, __println_newline
    mov edx, 1
    mov eax, 4
    mov ebx, 1
    int 0x80
    pop ebp
    ret

; println_char(value: char at [ebp+8]) -> void
println_char:
    push ebp
    mov ebp, esp
    sub esp, 16
    mov al, [ebp+8]
    mov [ebp-2], al
    mov byte [ebp-1], 10
    lea ecx, [ebp-2]
    mov edx, 2
    mov eax, 4
    mov ebx, 1
    int 0x80
    mov esp, ebp
    pop ebp
    ret

; println_double(value: double at [ebp+8],[ebp+12]) -> void
println_double:
    push ebp
    mov ebp, esp
    sub esp, 64
    fld qword [ebp+8]
    ftst
    fstsw ax
    sahf
    jae .Ldbl_positive
    mov ecx, __println_minus
    mov edx, 1
    mov eax, 4
    mov ebx, 1
    int 0x80
    fld qword [ebp+8]
    fchs
    jmp .Ldbl_abs_done
.Ldbl_positive:
    fld qword [ebp+8]
.Ldbl_abs_done:
    fistp qword [ebp-24]
    mov eax, [ebp-24]
    lea esi, [ebp-48]
    add esi, 20
    mov byte [esi], 0
    mov eax, [ebp-24]
    test eax, eax
    jnz .Ldbl_iconv
    dec esi
    mov byte [esi], '0'
    jmp .Ldbl_iwrite
.Ldbl_iconv:
.Ldbl_iloop:
    xor edx, edx
    mov ecx, 10
    div ecx
    add dl, '0'
    dec esi
    mov [esi], dl
    test eax, eax
    jnz .Ldbl_iloop
.Ldbl_iwrite:
    lea edx, [ebp-48]
    add edx, 20
    sub edx, esi
    mov ecx, esi
    mov eax, 4
    mov ebx, 1
    int 0x80
    fld qword [ebp+8]
    fabs
    fild dword [ebp-24]
    fsubp st1, st0
    mov eax, 1000000
    push eax
    fild dword [esp]
    add esp, 4
    fmulp st1, st0
    frndint
    fistp dword [ebp-32]
    mov eax, [ebp-32]
    test eax, eax
    jz .Ldbl_newline
    mov ecx, __println_dot
    mov edx, 1
    mov eax, 4
    mov ebx, 1
    int 0x80
    lea esi, [ebp-48]
    add esi, 20
    mov byte [esi], 0
    mov eax, [ebp-32]
    mov ecx, 6
.Ldbl_floop:
    xor edx, edx
    push ecx
    mov ecx, 10
    div ecx
    pop ecx
    add dl, '0'
    dec esi
    mov [esi], dl
    dec ecx
    jnz .Ldbl_floop
    lea ebx, [esi+5]
.Ldbl_strip:
    cmp ebx, esi
    jl .Ldbl_stripped
    cmp byte [ebx], '0'
    jne .Ldbl_stripped
    dec ebx
    jmp .Ldbl_strip
.Ldbl_stripped:
    mov edx, ebx
    sub edx, esi
    inc edx
    mov ecx, esi
    mov eax, 4
    mov ebx, 1
    int 0x80
.Ldbl_newline:
    mov ecx, __println_newline
    mov edx, 1
    mov eax, 4
    mov ebx, 1
    int 0x80
    add esp, 64
    mov esp, ebp
    pop ebp
    ret

; readln() -> string (ptr in eax)
readln:
    push ebp
    mov ebp, esp
    sub esp, 272
    mov ecx, [stdin_buf_len]
    cmp ecx, 0
    jg .Lreadln_has_buf
.Lreadln_read:
    xor eax, eax
    mov ebx, 0
    lea ecx, [esp]
    mov edx, 255
    int 0x80
    cmp eax, 0
    jle .Lreadln_empty
    xor ecx, ecx
.Lreadln_find_nl:
    cmp ecx, eax
    jge .Lreadln_no_nl
    cmp byte [esp + ecx], 10
    je .Lreadln_found_nl
    inc ecx
    jmp .Lreadln_find_nl
.Lreadln_found_nl:
    test ecx, ecx
    jz .Lreadln_do_copy
    cmp byte [esp + ecx - 1], 13
    jne .Lreadln_do_copy
    dec ecx
.Lreadln_do_copy:
    mov byte [esp + ecx], 0
    mov ebx, eax
    sub ebx, ecx
    dec ebx
    jle .Lreadln_copy
    mov [stdin_buf_len], ebx
    mov edi, stdin_buf
    lea esi, [esp + ecx + 1]
.Lreadln_save_loop:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    dec ebx
    jnz .Lreadln_save_loop
    jmp .Lreadln_copy
.Lreadln_no_nl:
    mov byte [esp + eax], 0
    mov ecx, eax
    xor ebx, ebx
    mov [stdin_buf_len], ebx
.Lreadln_copy:
    mov [ebp-8], ecx
    lea eax, [ecx + 1]
    push eax
    call heap_alloc
    add esp, 4
    mov [ebp-4], eax
    mov edx, eax
    mov ecx, [ebp-8]
    xor ebx, ebx
.Lreadln_cpy:
    cmp ebx, ecx
    jge .Lreadln_done
    mov al, [esp + ebx]
    mov [edx], al
    inc ebx
    inc edx
    jmp .Lreadln_cpy
.Lreadln_done:
    mov byte [edx], 0
    mov eax, [ebp-4]
    add esp, 272
    pop ebp
    ret
.Lreadln_has_buf:
    mov esi, stdin_buf
    xor ecx, ecx
.Lreadln_buf_find:
    cmp ecx, [stdin_buf_len]
    jge .Lreadln_buf_no_nl
    cmp byte [esi + ecx], 10
    je .Lreadln_buf_found
    inc ecx
    jmp .Lreadln_buf_find
.Lreadln_buf_found:
    mov [ebp-8], ecx
    lea eax, [ecx + 1]
    push eax
    call heap_alloc
    add esp, 4
    mov [ebp-4], eax
    mov edx, eax
    mov ecx, [ebp-8]
    xor ebx, ebx
.Lreadln_buf_cpy:
    cmp ebx, ecx
    jge .Lreadln_buf_cpy_done
    mov al, [esi + ebx]
    mov [edx], al
    inc ebx
    inc edx
    jmp .Lreadln_buf_cpy
.Lreadln_buf_cpy_done:
    mov byte [edx], 0
    mov ebx, [stdin_buf_len]
    mov ecx, [ebp-8]
    sub ebx, ecx
    dec ebx
    mov [stdin_buf_len], ebx
    jle .Lreadln_buf_ret
    mov edi, stdin_buf
    lea esi, [stdin_buf]
    add esi, [ebp-8]
    inc esi
    mov edx, ebx
.Lreadln_buf_shift:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    dec edx
    jnz .Lreadln_buf_shift
.Lreadln_buf_ret:
    mov eax, [ebp-4]
    add esp, 272
    pop ebp
    ret
.Lreadln_buf_no_nl:
    mov ecx, [stdin_buf_len]
    xor ebx, ebx
    mov [stdin_buf_len], ebx
    mov [ebp-8], ecx
    lea eax, [ecx + 1]
    push eax
    call heap_alloc
    add esp, 4
    mov [ebp-4], eax
    mov edx, eax
    mov ecx, [ebp-8]
    xor ebx, ebx
.Lreadln_buf_nl_cpy:
    cmp ebx, ecx
    jge .Lreadln_buf_nl_done
    mov al, [esi + ebx]
    mov [edx], al
    inc ebx
    inc edx
    jmp .Lreadln_buf_nl_cpy
.Lreadln_buf_nl_done:
    mov eax, [ebp-4]
    add esp, 272
    pop ebp
    ret
.Lreadln_empty:
    push 1
    call heap_alloc
    add esp, 4
    mov byte [eax], 0
    add esp, 272
    pop ebp
    ret

; __read_line helper: read next line into __read_line_buf, return ptr in esi
__read_line:
    push ebp
    mov ebp, esp
    sub esp, 32
    mov edi, __read_line_buf
    mov ecx, [stdin_buf_len]
    cmp ecx, 0
    jg __read_line_has_buf
__read_line_read:
    xor eax, eax
    mov ebx, 0
    mov ecx, __read_line_buf
    mov edx, 255
    int 0x80
    cmp eax, 0
    jle __read_line_empty
    xor ecx, ecx
    mov esi, __read_line_buf
__read_line_find:
    cmp ecx, eax
    jge __read_line_no_nl
    cmp byte [esi + ecx], 10
    je __read_line_found
    inc ecx
    jmp __read_line_find
__read_line_found:
    mov byte [esi + ecx], 0
    mov ebx, eax
    sub ebx, ecx
    dec ebx
    jle __read_line_done
    mov [stdin_buf_len], ebx
    mov edi, stdin_buf
    lea esi, [__read_line_buf]
    add esi, ecx
    inc esi
    mov edx, ebx
__read_line_save:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    dec edx
    jnz __read_line_save
    jmp __read_line_done
__read_line_no_nl:
    mov byte [esi + eax], 0
    xor ebx, ebx
    mov [stdin_buf_len], ebx
__read_line_done:
    mov esi, __read_line_buf
    add esp, 32
    pop ebp
    ret
__read_line_has_buf:
    mov esi, stdin_buf
    xor ecx, ecx
__read_line_buf_find:
    cmp ecx, [stdin_buf_len]
    jge __read_line_buf_all
    cmp byte [esi + ecx], 10
    je __read_line_buf_found
    inc ecx
    jmp __read_line_buf_find
__read_line_buf_found:
    mov [ebp-8], ecx
    mov edi, __read_line_buf
    xor ebx, ebx
__read_line_buf_cpy_loop:
    cmp ebx, ecx
    jge __read_line_buf_cpy_done
    mov al, [esi + ebx]
    mov [edi + ebx], al
    inc ebx
    jmp __read_line_buf_cpy_loop
__read_line_buf_cpy_done:
    mov byte [edi + ecx], 0
    mov ecx, [ebp-8]
    mov ebx, [stdin_buf_len]
    sub ebx, ecx
    dec ebx
    mov [stdin_buf_len], ebx
    jle __read_line_buf_ret
    mov edi, stdin_buf
    lea esi, [stdin_buf]
    add esi, [ebp-8]
    inc esi
    mov edx, ebx
__read_line_buf_shift:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    dec edx
    jnz __read_line_buf_shift
__read_line_buf_ret:
    mov esi, __read_line_buf
    add esp, 32
    pop ebp
    ret
__read_line_buf_all:
    mov ecx, [stdin_buf_len]
    xor ebx, ebx
    mov [stdin_buf_len], ebx
    mov edi, __read_line_buf
__read_line_buf_all_cpy:
    cmp ebx, ecx
    jge __read_line_buf_all_done
    mov al, [esi + ebx]
    mov [edi + ebx], al
    inc ebx
    jmp __read_line_buf_all_cpy
__read_line_buf_all_done:
    mov byte [edi + ecx], 0
    mov esi, __read_line_buf
    add esp, 32
    pop ebp
    ret
__read_line_empty:
    mov esi, __read_line_buf
    mov byte [esi], 0
    add esp, 32
    pop ebp
    ret

; read_int() -> int32 in eax
read_int:
    push ebp
    mov ebp, esp
    sub esp, 32
    call __read_line
    xor eax, eax
    xor ecx, ecx
    xor ebx, ebx
.read_int_skip:
    mov cl, [esi]
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
    inc esi
    jmp .read_int_skip
.read_int_neg:
    inc esi
    mov ebx, 1
    mov cl, [esi]
    cmp cl, '0'
    jb .read_int_zero
    cmp cl, '9'
    ja .read_int_zero
.read_int_loop:
    mov cl, [esi]
    cmp cl, 0
    je .read_int_done
    cmp cl, '0'
    jb .read_int_done
    cmp cl, '9'
    ja .read_int_done
    imul eax, 10
    movzx ecx, cl
    add eax, ecx
    sub eax, '0'
    inc esi
    jmp .read_int_loop
.read_int_done:
    cmp ebx, 1
    jne .read_int_ret
    neg eax
.read_int_ret:
    add esp, 32
    pop ebp
    ret
.read_int_zero:
    xor eax, eax
    add esp, 32
    pop ebp
    ret

; read_double() -> double (bits in edx:eax for 64-bit; caller may use st0 or slot)
read_double:
    push ebp
    mov ebp, esp
    sub esp, 32
    call __read_line
    xor ebx, ebx
    xor edi, edi
    xor ecx, ecx
    mov edx, 1
.readdbl_skip:
    mov al, [esi]
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
    inc esi
    jmp .readdbl_skip
.readdbl_neg:
    inc esi
    mov ecx, 1
    mov al, [esi]
    cmp al, '0'
    jb .readdbl_zero
    cmp al, '9'
    ja .readdbl_zero
.readdbl_int:
    mov al, [esi]
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
    imul ebx, 10
    movzx eax, al
    add ebx, eax
    sub ebx, '0'
    inc esi
    jmp .readdbl_int
.readdbl_frac:
    inc esi
    xor edi, edi
    mov edx, 1
.readdbl_frac_loop:
    mov al, [esi]
    cmp al, 0
    je .readdbl_conv
    cmp al, '0'
    jb .readdbl_conv
    cmp al, '9'
    ja .readdbl_conv
    imul edi, 10
    movzx eax, al
    add edi, eax
    sub edi, '0'
    imul edx, 10
    inc esi
    jmp .readdbl_frac_loop
.readdbl_conv:
    mov [ebp-8], ebx
    mov dword [ebp-4], 0
    fild qword [ebp-8]
    mov [ebp-8], edi
    mov dword [ebp-4], 0
    fild qword [ebp-8]
    mov [ebp-8], edx
    mov dword [ebp-4], 0
    fild qword [ebp-8]
    fdivp st1, st0
    faddp st1, st0
    cmp ecx, 1
    jne .readdbl_ret
    fchs
.readdbl_ret:
    fstp qword [ebp-16]
    mov eax, [ebp-16]
    mov edx, [ebp-12]
    add esp, 32
    pop ebp
    ret
.readdbl_zero:
    xor eax, eax
    xor edx, edx
    add esp, 32
    pop ebp
    ret

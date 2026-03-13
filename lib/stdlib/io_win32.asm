; picoC I/O standard library - Windows x86 (cdecl, underscore prefix)
BITS 32

global _println
global _println_double
global _println_str
global _println_bool
global _println_char
global _readln
global _read_int
global _read_double

global __println_fmt_int
global __println_fmt_double
global __println_fmt_str
global __println_fmt_char
global __println_bool_true
global __println_bool_false

extern _printf
extern _GetStdHandle@4
extern _ReadFile@20
extern _heap_alloc

section .data
    __println_fmt_int db "%d", 10, 0
    __println_fmt_double db "%g", 10, 0
    __println_fmt_str db "%s", 10, 0
    __println_fmt_char db "%c", 10, 0
    __println_bool_true db "true", 0
    __println_bool_false db "false", 0

section .text

; _println(value: int32 at [ebp+8]) -> void
_println:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push dword __println_fmt_int
    call _printf
    add esp, 8
    pop ebp
    ret

; _println_double(value: float at [ebp+8] - 32-bit codegen passes single float) -> void
_println_double:
    push ebp
    mov ebp, esp
    sub esp, 8
    mov eax, [ebp+8]
    mov [esp], eax
    fld dword [esp]
    fstp qword [esp]
    push dword __println_fmt_double
    call _printf
    add esp, 12
    mov esp, ebp
    pop ebp
    ret

; _println_str(ptr: char* at [ebp+8]) -> void
_println_str:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push dword __println_fmt_str
    call _printf
    add esp, 8
    pop ebp
    ret

; _println_bool(value: bool at [ebp+8]) -> void
_println_bool:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    cmp eax, 0
    je .Lbool_false
    push dword __println_bool_true
    jmp .Lbool_out
.Lbool_false:
    push dword __println_bool_false
.Lbool_out:
    push dword __println_fmt_str
    call _printf
    add esp, 8
    pop ebp
    ret

; _println_char(value: char at [ebp+8]) -> void
_println_char:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push dword __println_fmt_char
    call _printf
    add esp, 8
    pop ebp
    ret

; _readln() -> string (ptr in eax)
_readln:
    push ebp
    mov ebp, esp
    sub esp, 320
    push ebx
    push esi
    push edi
    push -10
    call _GetStdHandle@4
    mov ebx, eax
    lea ecx, [ebp-304]
    push 0
    push ecx
    push 255
    lea ecx, [ebp-296]
    push ecx
    push ebx
    call _ReadFile@20
    mov eax, [ebp-304]
    cmp eax, 0
    jle .Lreadln_empty
    xor ecx, ecx
.Lreadln_find_nl:
    cmp ecx, [ebp-304]
    jge .Lreadln_no_nl
    cmp byte [ebp-296+ecx], 10
    je .Lreadln_found_nl
    inc ecx
    jmp .Lreadln_find_nl
.Lreadln_found_nl:
    test ecx, ecx
    jz .Lreadln_do_copy
    cmp byte [ebp-296+ecx-1], 13
    jne .Lreadln_do_copy
    dec ecx
.Lreadln_do_copy:
    mov byte [ebp-296+ecx], 0
    jmp .Lreadln_copy
.Lreadln_no_nl:
    mov ecx, [ebp-304]
    mov byte [ebp-296+ecx], 0
.Lreadln_copy:
    inc ecx
    push ecx
    call _heap_alloc
    add esp, 4
    mov [ebp-308], eax
    mov edx, eax
    xor ecx, ecx
.Lreadln_cpy:
    cmp byte [ebp-296+ecx], 0
    je .Lreadln_done
    mov al, [ebp-296+ecx]
    mov [edx], al
    inc ecx
    inc edx
    jmp .Lreadln_cpy
.Lreadln_done:
    mov byte [edx], 0
    mov eax, [ebp-308]
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.Lreadln_empty:
    push 1
    call _heap_alloc
    add esp, 4
    mov byte [eax], 0
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; _read_int() -> int32 in eax
_read_int:
    push ebp
    mov ebp, esp
    sub esp, 344
    push ebx
    push esi
    push -10
    call _GetStdHandle@4
    lea ecx, [ebp-296]
    push 0
    push ecx
    push 255
    lea ecx, [ebp-288]
    push ecx
    push eax
    call _ReadFile@20
    mov eax, [ebp-296]
    cmp eax, 0
    jle .Lread_int_zero
    mov byte [ebp-288+eax], 0
    lea esi, [ebp-288]
    xor eax, eax
    xor ebx, ebx
.Lread_int_skip:
    mov cl, [esi]
    cmp cl, 0
    je .Lread_int_zero
    cmp cl, ' '
    je .Lread_int_skip_inc
    cmp cl, 9
    je .Lread_int_skip_inc
    cmp cl, 10
    je .Lread_int_skip_inc
    cmp cl, 13
    je .Lread_int_skip_inc
    cmp cl, '-'
    je .Lread_int_neg
    cmp cl, '0'
    jb .Lread_int_zero
    cmp cl, '9'
    ja .Lread_int_zero
    jmp .Lread_int_loop
.Lread_int_skip_inc:
    inc esi
    jmp .Lread_int_skip
.Lread_int_neg:
    inc esi
    mov ebx, 1
    mov cl, [esi]
    cmp cl, '0'
    jb .Lread_int_zero
    cmp cl, '9'
    ja .Lread_int_zero
.Lread_int_loop:
    mov cl, [esi]
    cmp cl, 0
    je .Lread_int_done
    cmp cl, '0'
    jb .Lread_int_done
    cmp cl, '9'
    ja .Lread_int_done
    imul eax, 10
    movzx ecx, cl
    add eax, ecx
    sub eax, '0'
    inc esi
    jmp .Lread_int_loop
.Lread_int_done:
    cmp ebx, 1
    jne .Lread_int_ret
    neg eax
.Lread_int_ret:
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.Lread_int_zero:
    xor eax, eax
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; _read_double() -> double bits in edx:eax (high:low)
_read_double:
    push ebp
    mov ebp, esp
    sub esp, 360
    push ebx
    push esi
    push edi
    push -10
    call _GetStdHandle@4
    lea ecx, [ebp-296]
    push 0
    push ecx
    push 255
    lea ecx, [ebp-288]
    push ecx
    push eax
    call _ReadFile@20
    mov eax, [ebp-296]
    cmp eax, 0
    jle .Lreaddbl_zero
    mov byte [ebp-288+eax], 0
    lea esi, [ebp-288]
    xor ebx, ebx
    xor edi, edi
    xor ecx, ecx
    mov edx, 1
.Lreaddbl_skip:
    mov al, [esi]
    cmp al, 0
    je .Lreaddbl_zero
    cmp al, ' '
    je .Lreaddbl_skip_inc
    cmp al, 9
    je .Lreaddbl_skip_inc
    cmp al, 10
    je .Lreaddbl_skip_inc
    cmp al, 13
    je .Lreaddbl_skip_inc
    cmp al, '-'
    je .Lreaddbl_neg
    cmp al, '0'
    jb .Lreaddbl_zero
    cmp al, '9'
    ja .Lreaddbl_zero
    jmp .Lreaddbl_int
.Lreaddbl_skip_inc:
    inc esi
    jmp .Lreaddbl_skip
.Lreaddbl_neg:
    inc esi
    mov ecx, 1
    mov al, [esi]
    cmp al, '0'
    jb .Lreaddbl_zero
    cmp al, '9'
    ja .Lreaddbl_zero
.Lreaddbl_int:
    mov al, [esi]
    cmp al, 0
    je .Lreaddbl_conv
    cmp al, ' '
    je .Lreaddbl_conv
    cmp al, 9
    je .Lreaddbl_conv
    cmp al, 10
    je .Lreaddbl_conv
    cmp al, '.'
    je .Lreaddbl_frac
    cmp al, '0'
    jb .Lreaddbl_conv
    cmp al, '9'
    ja .Lreaddbl_conv
    imul ebx, 10
    movzx eax, al
    add ebx, eax
    sub ebx, '0'
    inc esi
    jmp .Lreaddbl_int
.Lreaddbl_frac:
    inc esi
    xor edi, edi
    mov edx, 1
.Lreaddbl_frac_loop:
    mov al, [esi]
    cmp al, 0
    je .Lreaddbl_conv
    cmp al, '0'
    jb .Lreaddbl_conv
    cmp al, '9'
    ja .Lreaddbl_conv
    imul edi, 10
    movzx eax, al
    add edi, eax
    sub edi, '0'
    imul edx, 10
    inc esi
    jmp .Lreaddbl_frac_loop
.Lreaddbl_conv:
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
    jne .Lreaddbl_pos
    fchs
.Lreaddbl_pos:
    fstp qword [ebp-16]
    mov eax, [ebp-16]
    mov edx, [ebp-12]
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
.Lreaddbl_zero:
    xor eax, eax
    xor edx, edx
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

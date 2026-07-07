[BITS 64]

%define SYS_EXIT  0
%define SYS_WRITE 1

%define LOAD_VA 0x0000400000000000

    org LOAD_VA

ehdr:
    db 0x7F, 'E', 'L', 'F'
    db 2
    db 1
    db 1
    db 0
    times 8 db 0
    dw 2
    dw 0x3E
    dd 1
    dq _start
    dq phdr - $$
    dq 0
    dd 0
    dw 64
    dw 56
    dw 1
    dw 64
    dw 0
    dw 0

phdr:
    dd 1
    dd 5
    dq 0
    dq LOAD_VA
    dq LOAD_VA
    dq filesize
    dq filesize
    dq 0x1000

_start:
    lea rdi, [rel msg]
    mov rsi, msg_len
    mov rax, SYS_WRITE
    syscall

    xor rdi, rdi
    mov rax, SYS_EXIT
    syscall

    hlt

msg:    db "Hello from test.elf via syscall!", 0x0A
msg_len equ $ - msg

filesize equ $ - $$

;
; SPDX-License-Identifier: GPL-3.0-or-later
;
; Copyright (c) 2026 doccrLabs
;
; PROJECT: doccrOS
; FILE: usermode.asm
; CREATED BY: emex
; MODIFIED BY: Offihito
;
;

[BITS 64]

extern syscall_dispatch
extern syscall_scratch

global isr128
global syscall_entry

isr128:
    push 0
    push 128
    jmp syscall_common_stub

syscall_entry:
    mov [syscall_scratch + 0], rsp
    mov rsp, [syscall_scratch + 8]

    push qword 0x1b
    push qword [syscall_scratch + 0]
    push r11
    push qword 0x23
    push rcx

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call syscall_dispatch

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    pop rcx        
    add rsp, 8     
    pop r11       
    add rsp, 8     

    cli
    mov rsp, [syscall_scratch + 0]
    o64 sysret

syscall_common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call syscall_dispatch

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

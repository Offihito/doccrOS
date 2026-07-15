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
    push qword 0
    push qword 57

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
    pop rcx
    add rsp, 8
    pop r11

    cli
    ; RSP is part of this thread's saved syscall frame. Do not reload it
    ; from the global entry scratch slot: another thread may have used that
    ; slot while this syscall was suspended in SYS_YIELD.
    mov rsp, [rsp]
    o64 sysret

%define FORK_R15     88
%define FORK_R14     96
%define FORK_R13    104
%define FORK_R12    112
%define FORK_R11    120
%define FORK_R10    128
%define FORK_R9     136
%define FORK_R8     144
%define FORK_RBP    152
%define FORK_RDI    160
%define FORK_RSI    168
%define FORK_RDX    176
%define FORK_RCX    184
%define FORK_RBX    192
%define FORK_RAX    200
%define FORK_RIP    208
%define FORK_RFLAGS 216
%define FORK_RSP    224

global fork_child_return
fork_child_return:
    mov rcx, [r15 + FORK_RIP]
    mov r11, [r15 + FORK_RFLAGS]
    mov rsp, [r15 + FORK_RSP]
    mov rax, [r15 + FORK_RAX]
    mov rbx, [r15 + FORK_RBX]
    mov rdx, [r15 + FORK_RDX]
    mov rsi, [r15 + FORK_RSI]
    mov rdi, [r15 + FORK_RDI]
    mov rbp, [r15 + FORK_RBP]
    mov r8,  [r15 + FORK_R8]
    mov r9,  [r15 + FORK_R9]
    mov r10, [r15 + FORK_R10]
    mov r12, [r15 + FORK_R12]
    mov r13, [r15 + FORK_R13]
    mov r14, [r15 + FORK_R14]
    mov r15, [r15 + FORK_R15]
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

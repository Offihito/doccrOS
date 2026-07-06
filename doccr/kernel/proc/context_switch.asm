;
; SPDX-License-Identifier: GPL-3.0-or-later
;
; Copyright (c) 2026 doccrLabs
;
; PROJECT: doccrOS
; FILE: context_switch.asm
; CREATED BY: emex
; MODIFIED BY: --
;
;

[BITS 64]

global context_switch
global thread_trampoline
extern thread_exit          ; lives in thread.c, nasm cant read minds

global user_thread_trampoline
extern arch_enter_usermode

context_switch:
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    pushfq

    cli
    mov [rdi], rsp
    mov rsp, rsi

    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ret

thread_trampoline:
    mov rdi, r13
    call r12
    call thread_exit
.hang:
    cli
    hlt
    jmp .hang ; just in case thread_exit somehow comes back (it shouldnt)

user_thread_trampoline:
    mov rdi, r12
    mov rsi, r14
    mov rdx, r13
    call arch_enter_usermode ; does not return
.hang:
    cli
    hlt
    jmp .hang

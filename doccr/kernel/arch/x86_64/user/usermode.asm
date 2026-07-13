;
; SPDX-License-Identifier: GPL-3.0-or-later
;
; Copyright (c) 2026 doccrLabs
;
; PROJECT: doccrOS
; FILE: usermode.asm
; CREATED BY: emex
; MODIFIED BY: --
;
;

[BITS 64]

global arch_enter_usermode

; ok so it took me 2 months in emexOS i hope i can do it better this time TT
arch_enter_usermode:
    cli                 ; must stay off until iretq restores user RFLAGS
    mov rax, rdi ; entry point
    mov rcx, rsi ; user stack top
    mov rdi, rdx ; SysV first arg
    mov r11, 0x1B; user data selector (0x18 | RPL3)
    ;mov ds, r11w
    ;mov es, r11w
    ;mov fs, r11w
    ;mov gs, r11w

    push 0x1B ; SS
    push rcx ;  RSP
    pushfq
    pop r11
    or r11, 0x200 ; set IF so user code runs with interrupts on
    push r11   ; RFLAGS
    push 0x23  ; CS 0x20 | RPL3
    push rax   ; RIP

    ;mov ax, 0x10
    ;mov ds, ax
    ;mov es, ax
    ;mov fs, ax
    ;mov gs, ax

    iretq

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: syscall.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "syscall.h"
#include "sys_io.h"
#include "sys_process.h"
#include <kernel/arch/x86_64/idt/idt.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/screen/lib/print.h>

u64 syscall_scratch[2];

static inline void wrmsr(u32 msr, u64 val)
{
    __asm__ volatile(
        "wrmsr"
        :
        : "c"(msr), "a"((u32)(val & 0xFFFFFFFF)), "d"((u32)(val >> 32))
    );
}

static inline u64 rdmsr(u32 msr)
{
    u32 lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((u64)hi << 32) | lo;
}

static void syscall_enable(void)
{
    u64 efer = rdmsr(MSR_EFER);
    wrmsr(MSR_EFER, efer | EFER_SCE);

    u64 star = ((u64)KERNEL_CODE_SELECTOR << 32) |
               ((u64)KERNEL_DATA_SELECTOR << 48);
    wrmsr(MSR_STAR, star);

    wrmsr(MSR_LSTAR, (u64)syscall_entry);

    wrmsr(MSR_SFMASK, (1 << 9));
}

void syscall_install(void)
{
    idt_set_gate(128, (u64)isr128, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_GATE_INT);
    syscall_enable();
    log("[SYSCALL]", "INT 128 + SYSCALL/SYSRET enabled\n");
}

void syscall_update_kstack(u64 kstack_top)
{
    syscall_scratch[1] = kstack_top;
}

void syscall_dispatch(cpu_state_t *state)
{
    switch (state->rax)
    {
        case SYS_READ:    sys_read(state);    break;
        case SYS_WRITE:   sys_write(state);   break;
        case SYS_FORK:    sys_fork(state);    break;
        case SYS_EXIT:    sys_exit(state);    break;
        case SYS_YIELD:   sys_yield(state);   break;
        case SYS_GETPID:  sys_getpid(state);  break;
        default:
            state->rax = (u64)-1;
            break;
    }
}

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: syscall.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "syscall.h"
#include <kernel/arch/x86_64/idt/idt.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/proc/thread.h>
#include <kernel/screen/lib/print.h>

void syscall_install(void)
{
    idt_set_gate(128, (u64)isr128, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_GATE_INT);
}

static int user_ptr_ok(u64 ptr)
{
    return ptr >= VMM_BASE && ptr < VMM_LIMIT;
}

void syscall_dispatch(cpu_state_t *state)
{
    u64 num = state->rax;

    if (num == SYS_EXIT)
    {
        thread_exit(); //doesnt return
    }

    switch (num)
    {
        case SYS_WRITE:
        {
            const char *buf = (const char *)state->rdi;
            u64 len         = state->rsi;

            if (user_ptr_ok((u64)buf))
            {
                for (
                    u64 i = 0;
                    i < len && buf[i];
                    i++
                ) putchar(buf[i], white());
            }

            state->rax = len;
            break;
        }

        default:
            state->rax = (u64)-1;
            break;
    }
}

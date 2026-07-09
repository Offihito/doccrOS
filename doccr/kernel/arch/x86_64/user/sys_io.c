/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_io.c
 * CREATED BY: Offihito
 *
 */

#include "sys_io.h"
#include <kernel/screen/lib/print.h>
#include <kernel/arch/x86_64/drivers/ps2/keyboard/keyboard.h>

static int user_ptr_ok(u64 ptr)
{
    return ptr != 0 && ptr <= 0x00007FFFFFFFFFFFULL;
}

void sys_read(cpu_state_t *state)
{
    u64 fd  = state->rdi;
    char *buf = (char *)state->rsi;
    u64 len = state->rdx;

    // TODO: route through /dev/input/keyboard
    if (fd == 0 && user_ptr_ok((u64)buf))
    {
        u64 n = 0;
        while (n < len && keyboard_has_key())
            buf[n++] = keyboard_get_key();
        state->rax = n;
    }
    else
    {
        state->rax = (u64)-1;
    }
}

void sys_write(cpu_state_t *state)
{
    u64 fd        = state->rdi;
    const char *buf = (const char *)state->rsi;
    u64 len       = state->rdx;

    if ((fd == 1 || fd == 2) && user_ptr_ok((u64)buf))
    {
        for (u64 i = 0; i < len; i++) putchar(buf[i], white());
        state->rax = len;
    }
    else
    {
        state->rax = (u64)-1;
    }
}

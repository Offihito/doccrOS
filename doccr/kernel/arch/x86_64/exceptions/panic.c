/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: panic.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "panic.h"
#include <kernel/screen/graphics.h>
#include <kernel/screen/colors.h>

#define PANICSCREEN_COLOR 0xFFffffff
#define PANICSCREEN_BG_COLOR 0xFFff0000

__attribute__((noreturn)) void panic(const char *message)
{
    clear(PANICSCREEN_BG_COLOR);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", PANICSCREEN_COLOR);
    print("!!! --- KERNEL PANIC --- !!!", PANICSCREEN_COLOR);
    print("\n", PANICSCREEN_COLOR);

    if (message) {
        print(message, PANICSCREEN_COLOR);
        print("\n", PANICSCREEN_COLOR);
    }

    print("\nSystem halted.", PANICSCREEN_COLOR);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

__attribute__((noreturn)) void panic_exception(cpu_state_t *state, const char *message)
{
    clear(PANICSCREEN_BG_COLOR);
    // Disable interrupts
    __asm__ volatile("cli");

    print("\n", PANICSCREEN_COLOR);
    print("!!! PANIC !!!", PANICSCREEN_COLOR);
    print("\n", PANICSCREEN_COLOR);

    if (message) {
        char buf[128];
        str_copy(buf, "Exception: ");
        str_append(buf, message);
        print(buf, PANICSCREEN_COLOR);
        print("\n", PANICSCREEN_COLOR);
    }

    // Print exception details
    char buf[128];
    str_copy(buf, "INT: ");
    str_append_uint(buf, (u32)state->int_no);
    str_append(buf, " ERR: ");
    str_append_uint(buf, (u32)state->err_code);
    print(buf, PANICSCREEN_COLOR);
    print("\n", PANICSCREEN_COLOR);

    // page fault has VIP treatment xd
    if (state->int_no == 14) {
        u64 cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

        str_copy(buf, "CR2 (faulting addr): 0x");
        str_from_hex(buf + str_len(buf), cr2);
        print(buf, PANICSCREEN_COLOR);
        print("\n", PANICSCREEN_COLOR);

        str_copy(buf, "  present: ");
        str_append(buf, (state->err_code & 1) ? "yes" : "no (unmapped)");
        str_append(buf, ", write: ");
        str_append(buf, (state->err_code & 2) ? "yes" : "no");
        print(buf, PANICSCREEN_COLOR);
        print("\n", PANICSCREEN_COLOR);
    }

    str_copy(buf, "RIP: 0x");
    str_from_hex(buf + str_len(buf), state->rip);
    print(buf, PANICSCREEN_COLOR);
    print("\n", PANICSCREEN_COLOR);

    // Print RSP
    str_copy(buf, "RSP: 0x");
    str_from_hex(buf + str_len(buf), state->rsp);
    print(buf, PANICSCREEN_COLOR);
    print("\n\n", PANICSCREEN_COLOR);

    print("System halted.", PANICSCREEN_COLOR);

    // HALT
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

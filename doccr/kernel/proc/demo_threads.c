/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: demo_threads.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "demo_threads.h"
#include <kernel/screen/lib/print.h>
#include <kernel/screen/colors.h>
#include <kernel/proc/scheduler.h>

// the im bored, wake me if something happens fah thread
void idle_fn(void *arg)
{
    (void)arg;
    for (;;) {
        __asm__ volatile("sti; hlt");
    }
}

//just proves the scheduler actually alternates threads
void worker_fn(void *arg)
{
    (void)arg;
    u32 count = 0;

    for (;;) {
        print("worker tick ", white());
        printInt((int)count++, white());
        print("\n", white());

        sched_yield(); // sharing is caring
    }
}

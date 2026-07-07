/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: demo_threads.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "demo_threads.h"
#include <kernel/screen/lib/print.h>
#include <kernel/screen/colors.h>
#include <kernel/proc/scheduler.h>
#include <kernel/communication/serial.h>

// the im bored, wake me if something happens fah thread
void idle_fn(void *arg)
{
    (void)arg;
    for (;;)
    {
        __asm__ volatile("sti; hlt");
    }
}

//just proves the scheduler actually alternates threads
void worker_fn(void *arg)
{
    u64 id  = (u64)arg;
    u32 count = 0;

    for (;;)
    {
        if ((count % 50) == 0)
            printf("[worker-%llu] tick=%u\n", id, count);
        count++;

        sched_yield();
    }
}

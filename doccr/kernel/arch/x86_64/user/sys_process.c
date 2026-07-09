/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_process.c
 * CREATED BY: Offihito
 *
 */

#include "sys_process.h"
#include <kernel/proc/thread.h>
#include <kernel/proc/process.h>
#include <kernel/proc/scheduler.h>
#include <kernel/communication/serial.h>

void sys_exit(cpu_state_t *state)
{
    (void)state;
    thread_exit();
}

void sys_yield(cpu_state_t *state)
{
    sched_yield();
    state->rax = 0;
}

void sys_getpid(cpu_state_t *state)
{
    proc_t *p = process_get_current();
    state->rax = p ? p->pid : (u64)-1;
}

void sys_fork(cpu_state_t *state)
{
    proc_t *child = process_fork(state);
    state->rax = child ? child->pid : (u64)-1;
}

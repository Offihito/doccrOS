/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_process.c
 * CREATED BY: Offihito
 * MODIFIED BY: emex
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

void sys_waitpid(cpu_state_t *state)
{
    i64 target_pid = (i64)state->rdi;
    int *wstatus_ptr = (int *)state->rsi;

    proc_t *caller    = process_get_current();
    if (!caller)
    {
        state->rax    = (u64)-1;
        return;
    }

    int exit_code    = 0;
    int result       = process_waitpid(caller, target_pid, &exit_code);

    if (result != 0)
    {
        state->rax   = (u64)-1; // no dead kids found
        return;
    }


    if (
    	wstatus_ptr &&
     	(u64)wstatus_ptr <= 0x00007FFFFFFFFFFFULL
    )*wstatus_ptr  = (exit_code & 0xFF) << 8;

    state->rax     = (u64)target_pid;
}

//always 0 for now cuz were jst root
void sys_getuid(cpu_state_t *state)
{
    proc_t *p = process_get_current();
    state->rax = p ? (u64)p->uid : 0; // 0 = root, this is fine trust me
}

void sys_getgid(cpu_state_t *state)
{
    proc_t *p = process_get_current();
    state->rax = p ? (u64)p->gid : 0;
}

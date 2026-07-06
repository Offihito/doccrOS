/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: thread.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef THREAD_H
#define THREAD_H

#include <types.h>

#define     THREAD_STACK_SIZE 16384
#define     THREAD_NAME_MAX      32

typedef enum
{
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_DEAD
} thread_state_t;

typedef void  (*thread_entry_t)(void *arg);

struct     proc;

typedef struct thread {
    u64    tid;
    char    name[THREAD_NAME_MAX];
    thread_state_t  state;

    u64    rsp;         //saved stack ptr,
                        // only meaningful when NOT running
    u8     *stack_base;
    u64    stack_size;

    int    is_user;
    u64    kstack_top; //TSS.rsp0

    struct proc    *owner;           // which proc
    struct thread    *proc_next;     // next thread in the same proc
    struct thread    *sched_next;    // scratch link for  sched
} thread_t;

void   thread_subsystem_init(void);
void   thread_destroy(thread_t *t);

thread_t *thread_create(struct proc *owner, const char *name, thread_entry_t entry, void *arg);
thread_t *thread_create_user(struct proc *owner, const char *name, thread_entry_t entry,void *arg, u64 user_stack_top);
thread_t *thread_get_current(void);

__attribute__((noreturn)) void thread_exit(void);


#endif

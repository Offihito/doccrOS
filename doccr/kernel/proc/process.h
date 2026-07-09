/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: process.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <types.h>
#include "thread.h"
#include <kernel/mem/vmm/vmm.h>
#include <kernel/arch/x86_64/idt/idt.h>

typedef enum {
    PROC_ALIVE,
    PROC_ZOMBIE,
    PROC_DEAD
} proc_state_t;

typedef struct proc
{
    u64      pid;
    char     name[64];

    proc_state_t  state;
    int      exit_code;

    vmm_space_t  *space;

    thread_t     *threads;      // all threads living in this proc
    int    thread_count;
    int    alive_count;

    struct proc     *next;
} proc_t;

void process_init(void);
void process_destroy(proc_t *p);

proc_t *process_create(const char *name);
proc_t *process_create_user(const char *name);
proc_t *process_get_current(void);
proc_t *process_fork(cpu_state_t *parent_state);

void process_exit(proc_t *p, int exit_code);
void process_reap_zombies(void);

#endif

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: process.h
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <types.h>
#include "thread.h"
#include <kernel/mem/vmm/vmm.h>
#include <kernel/arch/x86_64/idt/idt.h>
#include <kernel/fs/vfs/vfs.h>

#define FD_MAX 64

typedef struct {
    vfs_node_t *node;
    u64         offset;
    int         used;
} file_descriptor_t;

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

    u32     uid;
    u32     gid;

    u64      heap_break;

    vmm_space_t  *space;

    thread_t     *threads;
    int    thread_count;
    int    alive_count;

    file_descriptor_t fd_table[FD_MAX];

    struct proc     *next;
} proc_t;

void process_init(void);
void process_destroy(proc_t *p);

proc_t *process_create(const char *name);
proc_t *process_create_user(const char *name);
proc_t *process_get_current(void);
proc_t *process_fork(cpu_state_t *parent_state);

int process_waitpid(proc_t *parent, i64 target_pid, int *exit_code_out);

void process_exit(proc_t *p, int exit_code);
void process_reap_zombies(void);

#endif

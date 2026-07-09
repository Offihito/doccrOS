/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_fs.c
 * CREATED BY: Offihito
 *
 */

#include "sys_fs.h"
#include <kernel/proc/process.h>
#include <kernel/fs/vfs/vfs.h>

static int user_ptr_ok(u64 ptr)
{
    return ptr != 0 && ptr <= 0x00007FFFFFFFFFFFULL;
}

void sys_open(cpu_state_t *state)
{
    const char *path = (const char *)state->rdi;
    u64 flags        = state->rsi;

    (void)flags;

    if (!user_ptr_ok((u64)path))
    {
        state->rax = (u64)-1;
        return;
    }

    vfs_node_t *node = vfs_find(path);
    if (!node)
    {
        state->rax = (u64)-1;
        return;
    }

    proc_t *p = process_get_current();
    if (!p)
    {
        state->rax = (u64)-1;
        return;
    }

    for (int fd = 3; fd < FD_MAX; fd++)
    {
        if (!p->fd_table[fd].used)
        {
            p->fd_table[fd].node   = node;
            p->fd_table[fd].offset = 0;
            p->fd_table[fd].used   = 1;
            state->rax = (u64)fd;
            return;
        }
    }

    state->rax = (u64)-1;
}

void sys_close(cpu_state_t *state)
{
    u64 fd = state->rdi;

    if (fd < 3 || fd >= FD_MAX)
    {
        state->rax = (u64)-1;
        return;
    }

    proc_t *p = process_get_current();
    if (!p)
    {
        state->rax = (u64)-1;
        return;
    }

    if (!p->fd_table[fd].used)
    {
        state->rax = (u64)-1;
        return;
    }

    p->fd_table[fd].node   = NULL;
    p->fd_table[fd].offset = 0;
    p->fd_table[fd].used   = 0;

    state->rax = 0;
}

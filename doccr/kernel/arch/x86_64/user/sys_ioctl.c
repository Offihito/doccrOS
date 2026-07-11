/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_ioctl.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "sys_ioctl.h"
#include <kernel/proc/process.h>
#include <kernel/fs/vfs/vfs.h>
#include <kernel/devices/device_init.h>

static int user_ptr_ok(u64 ptr)
{
    return ptr     != 0 && ptr <= 0x00007FFFFFFFFFFFULL;
}

void sys_ioctl(cpu_state_t *state)
{
    u64 fd         = state->rdi;
    u64 request    = state->rsi;
    void *arg      = (void *)state->rdx;

    if (fd < 3     || fd >= FD_MAX)
    {
        state->rax = (u64)-1;
        return;
    }

    // arg is optional
    if (arg &&     !user_ptr_ok((u64)arg))
    {
        state->rax = (u64)-1;
        return;
    }

    proc_t *p      = process_get_current();
    if (!p || !p->fd_table[fd].used)
    {
        state->rax = (u64)-1;
        return;
    }

    vfs_node_t *node   = p->fd_table[fd].node;
    if (!node || node->type != VFS_DEVICE)
    {
        state->rax = (u64)-1; // ioctl only makes sense on devices
        return;
    }

    if (!node->device  || !node->device->ioctl)
    {
        state->rax = (u64)-1;
        return;
    }

    state->rax = (u64)node->device->ioctl(
    	p->fd_table[fd].device_handle,
     	request,
     	arg
    );
}
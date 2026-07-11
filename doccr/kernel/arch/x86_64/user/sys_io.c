/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_io.c
 * CREATED BY: Offihito
 * MODIFIED BY: emex
 *
 */

#include "sys_io.h"
#include <kernel/screen/lib/print.h>
#include <kernel/arch/x86_64/drivers/ps2/keyboard/keyboard.h>
#include <kernel/communication/serial.h>
#include <kernel/proc/process.h>
#include <kernel/fs/vfs/vfs.h>
#include <kernel/devices/device_init.h>
#include <kernel/mem/lib.h>

static int user_ptr_ok(u64 ptr)
{
    return ptr != 0 && ptr <= 0x00007FFFFFFFFFFFULL;
}

void sys_read(cpu_state_t *state)
{
    u64 fd  = state->rdi;
    char *buf = (char *)state->rsi;
    u64 len = state->rdx;

    if (!user_ptr_ok((u64)buf))
    {
        state->rax = (u64)-1;
        return;
    }

    // TODO: route through /dev/input/keyboard
    if (fd    == 0)
    {
        u64 n = 0;
        while (
        	n < len &&
         	keyboard_has_key()) buf[n++] = keyboard_get_key();
        state->rax = n;
        return;
    }

    if (fd < 3 || fd >= FD_MAX)
    {
        state->rax = (u64)-1;
        return;
    }

    proc_t *p = process_get_current();
    if (!p    || !p->fd_table[fd].used)
    {
        state->rax = (u64)-1;
        return;
    }

    vfs_node_t *node = p->fd_table[fd].node;
    if (!node)
    {
        state->rax = (u64)-1;
        return;
    }

    if (node->type == VFS_DEVICE)
    {
        if (!node->device || !node->device->read)
        {
            state->rax = (u64)-1;
            return;
        }

        state->rax  = (u64)node->device->read(p->fd_table[fd].device_handle, buf, len);
        return;
    }

    if (node->type != VFS_FILE)
    {
        state->rax  = (u64)-1;
        return;
    }

    u64 offset  = p->fd_table[fd].offset;
    if (offset >= node->size)
    {
        state->rax = 0;
        return;
    }

    u64 remaining = node->size - offset;
    u64 to_copy   = (len < remaining) ? len : remaining;

    memcpy(buf, node->data + offset, to_copy);
    p->fd_table[fd].offset += to_copy;

    state->rax    = to_copy;
}

void sys_write(cpu_state_t *state)
{
    u64 fd        = state->rdi;
    const char *buf = (const char *)state->rsi;
    u64 len       = state->rdx;

    if (!user_ptr_ok((u64)buf))
    {
        state->rax = (u64)-1;
        return;
    }

    if (fd == 1 || fd == 2)
    {
        for (u64 i = 0; i < len; i++)
        {
            putchar(buf[i], white());
            serial_putchar(buf[i]);
        }
        state->rax = len;
        return;
    }

    if (fd < 3 || fd >= FD_MAX)
    {
        state->rax = (u64)-1;
        return;
    }

    proc_t *p = process_get_current();
    if (!p || !p->fd_table[fd].used)
    {
        state->rax = (u64)-1;
        return;
    }

    vfs_node_t *node = p->fd_table[fd].node;
    if (!node)
    {
        state->rax = (u64)-1;
        return;
    }

    if (node->type == VFS_DEVICE)
    {
        if (!node->device || !node->device->write)
        {
            state->rax = (u64)-1;
            return;
        }

        state->rax = (u64)node->device->write(p->fd_table[fd].device_handle, buf, len);
        return;
    }

    if (node->type != VFS_FILE)
    {
        state->rax  = (u64)-1;
        return;
    }

    int written  = vfs_write(node, buf, len);
    state->rax   = (written < 0) ? (u64)-1 : (u64)written;
}

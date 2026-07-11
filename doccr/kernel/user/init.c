/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: init.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "init.h"

#include <kernel/fs/vfs/vfs.h>
#include <kernel/packages/elf/elf.h>
#include <kernel/proc/process.h>
#include <kernel/screen/lib/print.h>
#include <kernel/screen/lib/log.h>
#include <kernel/communication/serial.h>

#define SYSTEM_PATH "/bin/hello.elf"
#define SYSTEM_NAME "hello.elf"
#define DESKTOP_PATH "/bin/test_graphics.elf"
#define DESKTOP_NAME "test_graphics.elf"

#define SYSCALL_TEST_PATH "/bin/syscall_test.elf"
#define SYSCALL_TEST_NAME "syscall_test.elf"

static void load_elf(const char *path, const char *name, u64 initial_caps)
{
    vfs_node_t *node = vfs_find(path);

    if (!node)
    {
        printf("[USER] could not find %s, skipping...\n", path);
        return;
    }

    if (node->type != VFS_FILE)
    {
        log("[USER]", "path is not a file, skipping...\n", warning);
        return;
    }

    if (!node->data || node->size == 0)
    {
        log("[USER]", "binary is empty, skipping...\n", warning);
        return;
    }

    printf("[USER] found, load '%s' <%llu bytes>\n", path, node->size);

    int rc = elf_load(node->data, node->size, name, initial_caps);
    if (rc != 0)
    {
        log("[USER]", "could not load binary...\n", warning);
        return;
    }

    log("[USER]", "loading was a success!\n", success);
}

void user_start(void)
{
    load_elf(SYSTEM_PATH, SYSTEM_NAME, 0);
    load_elf(DESKTOP_PATH, DESKTOP_NAME, CAP_FRAMEBUFFER);
}
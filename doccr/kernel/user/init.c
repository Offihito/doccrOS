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
#include <kernel/screen/lib/print.h>
#include <kernel/screen/lib/log.h>
#include <kernel/communication/serial.h>

#define SYSTEM_PATH "/bin/hello.elf"
#define SYSTEM_NAME "hello.elf"

#define SYSCALL_TEST_PATH "/bin/syscall_test.elf"
#define SYSCALL_TEST_NAME "syscall_test.elf"

void user_start(void)
{
    vfs_node_t *node = vfs_find(SYSTEM_PATH);

    if (!node) {
        log("[USER]", "could not find " SYSTEM_PATH ", returning...\n", warning);
        return;
    }

    if (node->type != VFS_FILE) {
        log("[USER]", "path is not a file, returning...\n", warning);
        return;
    }

    if (!node->data || node->size == 0) {
        log("[USER]", "system init is empty, returning...\n", warning);
        return;
    }

    printf("[USER] found, load '%s' <%llu bytes>\n", SYSTEM_PATH, node->size);

    int rc = elf_load(node->data, node->size, SYSTEM_NAME);
    if (rc != 0) {
        log("[USER]", "could not load emxrc...\n", warning);
        return;
    }

    log("[USER]", "loading was a success!\n", success);
}
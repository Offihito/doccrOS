/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: rootfs.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "vfs.h"
#include <kernel/limine/reqs.h>
#include <kernel/packages/cpio/cpio.h>
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/print.h>

#define ROOTFS "initrd.cpio"

void rootfs_init(void)
{
    log("[VFS]", "setting up rootfs\n");

    vfs_mkdir("/dev");
    vfs_mkdir("/tmp");

    if (
        module_request.response == NULL ||
        module_request.response->module_count == 0
    ) {
        log("[VFS]", "no boot modules found, rootfs stays empty and lonely\n");
        return;
    }

    for (u64 i = 0; i < module_request.response->module_count; i++)
    {
        struct limine_file *mod = module_request.response->modules[i];
        if (!mod || !mod->path) continue;

        if (str_contains(mod->path, ROOTFS))
        {
            log("[VFS]", "found initrd.cpio\n");
            cpio_extract(mod->address, mod->size);
            return;
        }
    }

    log("[VFS]", "initrd.cpio was not in the modules, sadly :/\n");
}

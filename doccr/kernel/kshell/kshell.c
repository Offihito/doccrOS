/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: kshell.c
 * CREATED BY: Offihito
 *
 */

#include "kshell.h"

#include <kernel/arch/x86_64/drivers/ps2/keyboard/keyboard.h>
#include <kernel/screen/bootscreen/boot.h>
#include <kernel/screen/lib/print.h>
#include <kernel/screen/lib/string.h>
#include <kernel/screen/colors.h>
#include <kernel/communication/serial.h>
#include <kernel/proc/process.h>
#include <kernel/proc/thread.h>
#include <kernel/proc/scheduler.h>
#include <kernel/fs/vfs/vfs.h>
#include <kernel/mem/meminclude.h>
#include <kernel/packages/elf/elf.h>

#define KSHELL_BUF_MAX 256
#define KSHELL_PROMPT  "ksh> "


static void ksh_print(const char *s, u32 col)
{
    print(s, col);
}

static void ksh_newline(void)
{
    ksh_print("\n", white());
}

static void ksh_prompt(void)
{
    ksh_print(KSHELL_PROMPT, cyan());
}

static void cmd_clear(void)
{
    bs.Clear(BS1);
    printf("[ksh] screen cleared\n");
}

static void cmd_ls(const char *path)
{
    if (!path || path[0] == '\0')
        path = "/";

    vfs_node_t *node = vfs_find(path);
    if (!node) {
        ksh_print("ls: not found: ", red());
        ksh_print(path, red());
        ksh_newline();
        return;
    }

    if (node->type == VFS_FILE) {
        ksh_print(node->name, white());
        ksh_newline();
        return;
    }

    for (int i = 0; i < node->child_count; i++) {
        vfs_node_t *c = node->children[i];
        if (c->type == VFS_DIRECTORY) {
            ksh_print(c->name, cyan());
            ksh_print("/", cyan());
        } else {
            ksh_print(c->name, white());
        }
        ksh_newline();
    }
}

static void cmd_exec(const char *path)
{
    if (!path || path[0] == '\0') {
        ksh_print("exec: missing path\n", red());
        return;
    }

    vfs_node_t *node = vfs_find(path);
    if (!node) {
        ksh_print("exec: file not found: ", red());
        ksh_print(path, red());
        ksh_newline();
        return;
    }

    if (node->type != VFS_FILE) {
        ksh_print("exec: not a file: ", red());
        ksh_print(path, red());
        ksh_newline();
        return;
    }

    if (!node->data || node->size == 0) {
        ksh_print("exec: file is empty\n", red());
        return;
    }

    const char *pname = path;
    for (const char *p = path; *p; p++)
        if (*p == '/') pname = p + 1;

    printf("[ksh] exec '%s' (%llu bytes)\n", pname, node->size);

    int rc = elf_load(node->data, node->size, pname);
    if (rc != 0) {
        ksh_print("exec: failed to load ELF\n", red());
    } else {
        ksh_print("exec: scheduled '", green());
        ksh_print(pname, green());
        ksh_print("'\n", green());
    }
}

static void dispatch(char *line)
{
    while (*line == ' ') line++;

    if (line[0] == '\0')
        return;

    if (str_equals(line, "clear")) {
        cmd_clear();
        return;
    }
    if (str_starts_with(line, "exec")) {
        const char *arg = line + 4;
        while (*arg == ' ') arg++;
        cmd_exec(arg);
        return;
    }
    if (str_starts_with(line, "ls")) {
        const char *arg = line + 2;
        while (*arg == ' ') arg++;
        cmd_ls(arg);
        return;
    }

    ksh_print("unknown command: ", yellow());
    ksh_print(line, yellow());
    ksh_newline();
}

void shell_fn(void *arg)
{
    (void)arg;

    for (volatile int i = 0; i < 5000000; i++) __asm__ volatile("pause");

    ksh_print("\n[kshell] ring-0 shell ready\n", cyan());
    ksh_print("commands: clear  exec <vfs-path>  ls [vfs-path]\n\n", gray60());

    char buf[KSHELL_BUF_MAX];
    int  len = 0;

    ksh_prompt();

    for (;;)
    {
        if (!keyboard_has_key()) {
            sched_yield();
            continue;
        }

        char c = keyboard_get_key();

        if (c == '\n') {
            buf[len] = '\0';
            printf("\n");
            dispatch(buf);
            len = 0;
            ksh_prompt();
            continue;
        }

        if (c == '\b') {
            if (len > 0) {
                len--;
            }
            continue;
        }

        if (len < KSHELL_BUF_MAX - 1) {
            buf[len++] = c;
            printf("%c", c);
        }
    }
}

void kshell_init(void *kproc)
{
    proc_t *p = (proc_t *)kproc;
    thread_create(p, "kshell", shell_fn, NULL);
    printf("[kshell] shell thread created\n");
}

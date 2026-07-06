/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: process.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "process.h"
#include <kernel/mem/meminclude.h>
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/print.h>

#define STACK_SIZE 8192

static proc_t *head;
//static proc_t *current;
static proc_t *proc_zombies;
static u64 next_pid;

void process_init(void) {
    head = NULL;
    //current = NULL;
    proc_zombies = NULL;
    next_pid =    1;
    thread_subsystem_init();

    log("[PROC]", "Process manager\n");
}

static proc_t *proc_alloc(const char *name)
{
    proc_t *p = (proc_t *)kcalloc(1, sizeof(proc_t));
    if (!p) return NULL;
/*
    u64 stk = (u64)kmalloc(STACK_SIZE);
    if (!stk) {
        kfree((u64 *)p);
        return NULL;
    }*/

    p->pid = next_pid++;
    p->state = PROC_ALIVE;
    p->exit_code    = 0;
    p->threads = NULL;
    p->thread_count = 0;
    p->alive_count  = 0;
    p->next = head;

    int i =    0;
    if (name)
    {
        while (name[i] && i < 63)
        {
            p->name[i] = name[i];
            i++;
        }
    }

    p->name[i] =  '\0';

    head =    p;

    return p;
}

proc_t *process_create(const char *name)
{
    proc_t *p = proc_alloc(name);
    if (!p) return NULL;

    p->space = vmm_get_kernel_space(); // no extra pml4
    return p;
}

proc_t *process_create_user(const char *name)
{
    proc_t *p = proc_alloc(name);
    if (!p) return NULL;

    p->space = vmm_space_create();
    if (!p->space)
    {
        head = p->next;

        kfree((u64 *)p);
        return NULL;
    }
    return p;
}

void process_exit(proc_t *p, int exit_code)
{
    if (!p || p->state != PROC_ALIVE) return;

    p->exit_code = exit_code;
    p->state     = PROC_ZOMBIE;

    proc_t *cur  = head, *prev   = NULL;

    while (cur)
    {
        if (cur == p)
        {
            if (prev) prev->next = cur->next;
            else head     = cur->next;
            break;
        }
        prev     = cur;
        cur      = cur->next;
    }

    p->next = proc_zombies;
    proc_zombies = p;
}

void process_reap_zombies(void)
{
    while (proc_zombies)
    {
        proc_t *p     = proc_zombies;
        proc_zombies  = p->next;

        if (
            p->space && p->space != vmm_get_kernel_space()
        ) vmm_space_destroy(p->space); // pml4 + private frames

        p->state      = PROC_DEAD;
        kfree((u64 *)p);
    }
}

void process_destroy(proc_t *p)
{
    if (!p) return;

    thread_t *t = p->threads;
    while (t) {
        thread_t *next = t->proc_next;
        thread_destroy(t);
        t = next;
    }
    if (
        p->space && p->space != vmm_get_kernel_space()
    )vmm_space_destroy(p->space);

    proc_t *cur = head, *prev = NULL;

    while (cur)
    {
        if (cur == p)
        {
            if (prev) prev->next = cur->next;
            else head = cur->next;

            //if (current == p) current = NULL;

            //kfree((u64 *)p->stack_base);
            kfree((u64 *)p);
            return;
        }

        prev    = cur;
        cur     = cur->next;
    }
}

proc_t *process_get_current(void)
{
    thread_t *t =    thread_get_current();

    return t ? t->owner  : NULL;
}

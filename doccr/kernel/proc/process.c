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
static u64 next_pid;

void process_init(void) {
    head = NULL;
    //current = NULL;
    next_pid =    1;
    thread_subsystem_init();

    log("[PROC]", "Process manager\n");
}

proc_t *process_create(const char *name)
{
    proc_t *p = (proc_t *)kmalloc(sizeof(proc_t));
    if (!p) return NULL;
/*
    u64 stk = (u64)kmalloc(STACK_SIZE);
    if (!stk) {
        kfree((u64 *)p);
        return NULL;
    }*/

    p->pid = next_pid++;
    p->state = PROC_ALIVE;
    p->threads = NULL;
    p->thread_count = 0;
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

void process_destroy(proc_t *p)
{
    if (!p) return;

    thread_t *t = p->threads;
    while (t) {
        thread_t *next = t->proc_next;
        thread_destroy(t);
        t = next;
    }
    proc_t *cur = head, *prev = NULL;

    while (cur) {
        if (cur == p) {
            if (prev) prev->next = cur->next;
            else head = cur->next;

            //if (current == p) current = NULL;

            //kfree((u64 *)p->stack_base);
            kfree((u64 *)p);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

proc_t *process_get_current(void)
{
    thread_t *t =    thread_get_current();

    return t ? t->owner  : NULL;
}

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: scheduler.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "scheduler.h"
#include "process.h"
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/print.h>
#include <kernel/communication/serial.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/arch/x86_64/user/syscall.h>

#define QUANTUM 10

extern void context_switch(u64 *old_rsp_out, u64 new_rsp);

typedef struct {
    thread_t *queue[SCHED_MAX_THREADS];
    int head, tail, cnt;
} rq_t;

static rq_t ready_q;
static thread_t bootstrap;   // stands in for the og _start() flow before any real thread exists
static thread_t *current;
static thread_t *zombies;    // dead threads waiting to get properly buried
static int enabled;
static u64 ticks;

static u64 switch_count = 0;

static void q_init(rq_t *q)
{
    q->head = q->tail = q->cnt = 0;
}

static int q_empty(rq_t *q)
{
    return q->cnt == 0;
}

static void q_push(rq_t *q, thread_t *t)
{
    if (q->cnt >= SCHED_MAX_THREADS) return; // ready queue is full sucks to be you

    q->queue[q->tail] = t;
    q->tail = (q->tail + 1) % SCHED_MAX_THREADS;
    q->cnt++;
}

static thread_t *q_pop(rq_t *q)
{
    if (q_empty(q)) return NULL;

    thread_t *t = q->queue[q->head];

    q->head = (q->head + 1) % SCHED_MAX_THREADS;
    q->cnt--;

    return t;
}

void sched_init(void)
{
    q_init(&ready_q);

    bootstrap.tid   = 0;
    str_copy(bootstrap.name, "kernel_init");
    bootstrap.state = THREAD_RUNNING;
    bootstrap.owner = NULL;
    bootstrap.sched_next = NULL;

    current     = &bootstrap;
    zombies     = NULL;
    enabled     = 0;
    ticks     = 0;

    log("[SCHED]", "Scheduler init\n");
}

void sched_enable(void)
{
    enabled = 1;
}
void sched_disable(void)
{
    enabled = 0;
}
int sched_is_enabled(void)
{
    return enabled;
}
thread_t *sched_current(void)
{
    return current;
}
void sched_add(thread_t *t) {
    if (!t) return;
    t->state = THREAD_READY;
    q_push(&ready_q, t);
}

void sched_remove(thread_t *t)
{
    if (!t) return;

    for (int i = 0; i < ready_q.cnt; i++)
    {
        int idx = (ready_q.head + i) % SCHED_MAX_THREADS;

        if (ready_q.queue[idx] == t)
        {
            for (int j = i; j < ready_q.cnt - 1; j++)
            {
                int a = (ready_q.head + j) % SCHED_MAX_THREADS;
                int b = (ready_q.head + j + 1) % SCHED_MAX_THREADS;
                ready_q.queue[a] = ready_q.queue[b];
            }

            ready_q.cnt--;
            ready_q.tail = (ready_q.tail - 1 + SCHED_MAX_THREADS) % SCHED_MAX_THREADS;
            return;
        }
    }
}

// safe when noone is running
static void reap_zombies(void)
{
    while (zombies)
    {
        thread_t *z = zombies;
        zombies = z->sched_next;
        thread_destroy(z);
    }
}

void sched_tick(void)
{
    if (!enabled) return;

    ticks++;

    if (ticks % QUANTUM == 0)
    {
        sched_yield();
    }
}

u64 sched_get_ticks(void) { return ticks; }
u64 sched_get_switch_count(void) { return switch_count; }

void sched_yield(void) {
    if (!enabled) return;

    reap_zombies();
    process_reap_zombies();

    thread_t *prev     = current;
    thread_t *next     = q_pop(&ready_q);

    if (prev->state == THREAD_DEAD)
    {
        prev->sched_next     = zombies;
        zombies     = prev;

    } else if (prev->state == THREAD_RUNNING)
    {
        prev->state     = THREAD_READY;
        q_push(&ready_q, prev);
    }

    if (!next)
    {
        // nobody else wants to run, just keep going if we're still alive
        if (prev->state != THREAD_DEAD)
        {
            current     = prev;
            prev->state     = THREAD_RUNNING;
        }
        return;
    }

    switch_count++;
    printf(
        "[SCHED] #%llu tick=%llu: '%s'(tid=%llu) -> '%s'(tid=%llu)\n",
        switch_count,
        ticks,
        prev->name,
        prev->tid,
        next->name,
        next->tid
    );

    //log("sched", "yield now");

    next->state     = THREAD_RUNNING;
    current     = next;

    printf("[SCHED] before activate: next='%s' owner=%p\n", next->name, (void*)next->owner);

    if (next->owner && next->owner->space)
    {
        printf("[SCHED] activating space=%p pml4_phys=0x%llx\n",
               (void*)next->owner->space, next->owner->space->pml4_phys);
        vmm_space_activate(next->owner->space);
    }

    printf("[SCHED] is_user=%d kstack_top=0x%llx\n", next->is_user, next->kstack_top);


    __asm__ volatile("cli" ::: "memory");

    if (next->is_user)
    {
        gdt_set_kernel_stack(next->kstack_top);
        syscall_update_kstack(next->kstack_top);
    }

    printf("[SCHED] about to context_switch\n");

    context_switch(&prev->rsp, next->rsp);
}

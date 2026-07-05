/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: scheduler.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "thread.h"
#include <types.h>

thread_t *sched_current(void);

#define SCHED_MAX_THREADS 64

void sched_init(void);
void sched_enable(void);
void sched_disable(void);
int sched_is_enabled(void);

void sched_add(thread_t *t);
void sched_remove(thread_t *t);
void sched_tick(void);
void sched_yield(void);

// just to c if it works
u64 sched_get_ticks(void);
u64 sched_get_switch_count(void);

#endif

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: user_demo.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef USER_DEMO_H
#define USER_DEMO_H

#include <kernel/proc/thread.h>
extern thread_t *g_debug_canary_thread;

void user_demo_init(void);
#endif

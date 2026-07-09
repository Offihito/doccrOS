/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_process.h
 * CREATED BY: Offihito
 * MODIFIED BY: emex
 *
 */

#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>

void sys_exit(cpu_state_t *state);
void sys_yield(cpu_state_t *state);
void sys_getpid(cpu_state_t *state);
void sys_fork(cpu_state_t *state);
void sys_waitpid(cpu_state_t *state);
void sys_getuid(cpu_state_t *state);
void sys_getgid(cpu_state_t *state);

#endif

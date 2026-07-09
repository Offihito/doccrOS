/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_mem.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef SYS_MEM_H
#define SYS_MEM_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>

void sys_mmap(cpu_state_t *state);
void sys_munmap(cpu_state_t *state);
void sys_brk(cpu_state_t *state);

#endif

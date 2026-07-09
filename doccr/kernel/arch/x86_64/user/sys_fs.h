/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_fs.h
 * CREATED BY: Offihito
 *
 */

#ifndef SYS_FS_H
#define SYS_FS_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>

void sys_open(cpu_state_t *state);
void sys_close(cpu_state_t *state);

#endif

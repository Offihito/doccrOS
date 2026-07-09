/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_io.h
 * CREATED BY: Offihito
 *
 */

#ifndef SYS_IO_H
#define SYS_IO_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>

void sys_read(cpu_state_t *state);
void sys_write(cpu_state_t *state);

#endif

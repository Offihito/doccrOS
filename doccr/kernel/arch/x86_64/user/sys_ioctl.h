/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_ioctl.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef SYS_IOCTL_H
#define SYS_IOCTL_H

#include <types.h>
#include <kernel/arch/x86_64/idt/idt.h>

void sys_ioctl(cpu_state_t *state);

#endif
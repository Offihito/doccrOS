/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: syscall.h
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <types.h>
#include "../idt/idt.h"

#define SYS_READ            0
#define SYS_WRITE           1
#define SYS_OPEN            2
#define SYS_CLOSE           3
#define SYS_IOCTL          16
#define SYS_LSEEK           8
#define SYS_MMAP            9
#define SYS_MUNMAP         11
#define SYS_BRK            12
#define SYS_FORK           57
#define SYS_EXIT           60
#define SYS_GETPID         39
#define SYS_YIELD          24
#define SYS_WAITPID        61
#define SYS_GETDENTS       78
#define SYS_MKDIR          83
#define SYS_UNLINK         87
#define SYS_GETUID        102
#define SYS_GETGID        104

#define MSR_EFER       0xC0000080
#define MSR_STAR       0xC0000081
#define MSR_LSTAR      0xC0000082
#define MSR_SFMASK     0xC0000084

#define EFER_SCE (1ULL << 0)

extern void isr128(void);
extern void syscall_entry(void);

extern u64 syscall_scratch[2];

void syscall_install(void);
void syscall_update_kstack(u64 kstack_top);
void syscall_dispatch(cpu_state_t *state);

#endif

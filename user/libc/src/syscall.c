/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS userspace
 * FILE: syscall.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include <unistd.h>

static inline long syscall3(long num, long a1, long a2, long a3)
{
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long write(int fd, const void *buf, size_t count)
{
    return syscall3(SYS_WRITE, fd, (long)buf, (long)count);
}

long read(int fd, void *buf, size_t count)
{
    return syscall3(SYS_READ, fd, (long)buf, (long)count);
}

long open(const char *path, int flags)
{
    return syscall3(SYS_OPEN, (long)path, flags, 0);
}

long close(int fd)
{
    return syscall3(SYS_CLOSE, fd, 0, 0);
}

long getpid(void)
{
    return syscall3(SYS_GETPID, 0, 0, 0);
}

long fork(void)
{
    return syscall3(SYS_FORK, 0, 0, 0);
}

void yield(void)
{
    syscall3(SYS_YIELD, 0, 0, 0);
}

__attribute__((noreturn)) void _exit(int code)
{
    syscall3(SYS_EXIT, code, 0, 0);
    for (;;) __asm__ volatile("hlt");
}
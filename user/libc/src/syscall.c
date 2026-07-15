/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
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

static inline long syscall6(
    long num,
    long a1,
    long a2,
    long a3,
    long a4,
    long a5,
    long a6
){
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;

    __asm__ volatile
    (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3),
          "r"(r10), "r"(r8), "r"(r9)
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

long ioctl(int fd, unsigned long request, void *arg)
{
    return syscall3(SYS_IOCTL, fd, (long)request, (long)arg);
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

long lseek(int fd, long offset, int whence)
{
    return syscall3(SYS_LSEEK, fd, offset, whence);
}
long waitpid(long pid, int *wstatus, int options)
{
    return syscall3(SYS_WAITPID, pid, (long)wstatus, options);
}
long getuid(void)
{
    return syscall3(SYS_GETUID, 0, 0, 0);
}

long getgid(void)
{
    return syscall3(SYS_GETGID, 0, 0, 0);
}

long mkdir(const char *path, int mode)
{
    return syscall3(SYS_MKDIR, (long)path, mode, 0);
}
long unlink(const char *path)
{
    return syscall3(SYS_UNLINK, (long)path, 0, 0);
}
long getdents(int fd, void *buf, size_t size)
{
    return syscall3(SYS_GETDENTS, fd, (long)buf, (long)size);
}

void *brk_call(void *addr)
{
    long result = syscall3(SYS_BRK, (long)addr, 0, 0);
    return (void *)result;
}
void *mmap(
	void *addr,
	size_t length,
	int prot,
	int flags,
	int fd,
	long offset
)
{
    return (void *)syscall6
    (
        SYS_MMAP,
        (long)addr,
        (long)length,
        prot,
        flags,
        fd,
        offset
    );
}


long munmap(void *addr, size_t length)
{
    return syscall3(SYS_MUNMAP, (long)addr, (long)length, 0);
}

int clock_gettime(int clock_id, struct timespec *timespec)
{
    return (int)syscall3(SYS_CLOCK_GETTIME, clock_id, (long)timespec, 0);
}

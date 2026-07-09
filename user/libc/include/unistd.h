#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

#define SYS_EXIT        0
#define SYS_WRITE       1
#define SYS_READ        2
#define SYS_YIELD       3
#define SYS_GETPID      4

long write(int fd, const void *buf, size_t count);
long read(int fd, void *buf, size_t count);
long getpid(void);
void yield(void);

__attribute__((noreturn)) void _exit(int code);

#endif
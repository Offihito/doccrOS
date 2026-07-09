#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

#define SYS_READ        0
#define SYS_WRITE       1
#define SYS_FORK        57
#define SYS_EXIT        60
#define SYS_GETPID      39
#define SYS_YIELD       24

long write(int fd, const void *buf, size_t count);
long read(int fd, void *buf, size_t count);
long getpid(void);
long fork(void);
void yield(void);

__attribute__((noreturn)) void _exit(int code);

#endif

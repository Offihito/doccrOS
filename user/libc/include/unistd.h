#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

#define SYS_READ          0
#define SYS_WRITE         1
#define SYS_OPEN          2
#define SYS_CLOSE         3
#define SYS_FORK         57
#define SYS_EXIT         60
#define SYS_GETPID       39
#define SYS_YIELD        24
#define SYS_LSEEK         8
#define SYS_MMAP          9
#define SYS_MUNMAP       11
#define SYS_BRK          12
#define SYS_WAITPID      61
#define SYS_GETDENTS     78
#define SYS_MKDIR        83
#define SYS_UNLINK       87
#define SYS_GETUID      102
#define SYS_GETGID      104

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

long write(int fd, const void *buf, size_t count);
long read(int fd, void *buf, size_t count);
long open(const char *path, int flags);
long close(int fd);
long lseek(int fd, long offset, int whence);
long getpid(void);
long fork(void);
void yield(void);
long waitpid(long pid, int *wstatus, int options);
long getuid(void);
long getgid(void);
long mkdir(const char *path, int mode);
long unlink(const char *path);
long getdents(int fd, void *buf, size_t size);
void *brk_call(void *addr);

__attribute__((noreturn)) void _exit(int code);

#endif

#ifndef SYSCALL_H
#define SYSCALL_H

#include <types.h>
#include "../idt/idt.h"

#define SYS_EXIT       0
#define SYS_WRITE      1

extern void isr128(void);

void syscall_install(void);
void syscall_dispatch(cpu_state_t *state);

#endif

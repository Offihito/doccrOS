#pragma once
#include <types.h>

typedef struct cpu_state cpu_state_t;

typedef void (*irq_handler_t)(cpu_state_t *state);

void interrupts_install(void);
void interrupts_enable(void);
void interrupts_disable(void);

void irq_register_handler(u8 irq, irq_handler_t handler);
void irq_unregister_handler(u8 irq);

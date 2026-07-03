#include <kernel/arch/hal/cpu.h>
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "exceptions/irq.h"

void cpu_early_init(void)
{
    gdt_init();
    idt_init();
}

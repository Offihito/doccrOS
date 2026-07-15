#include <kernel/arch/hal/cpu.h>
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "exceptions/irq.h"

void cpu_early_init(void)
{
    unsigned long cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1UL << 2); /* CR0.EM: allow x87/SSE instructions. */
    cr0 |=  (1UL << 1); /* CR0.MP: make WAIT honor task-switched. */
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1UL << 9) | (1UL << 10); /* OSFXSR | OSXMMEXCPT */
    __asm__ volatile("mov %0, %%cr4" :: "r"(cr4) : "memory");
    __asm__ volatile("fninit");
    gdt_init();
    idt_init();
}

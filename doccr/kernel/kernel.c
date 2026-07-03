/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: kernel.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "kernel.h"

#include <kernel/arch/hal/assembly.h>
#include <kernel/limine/reqs.h>
#include <kernel/include/logo.h>
#include <kernel/communication/serial.h>

#include <kernel/screen/graphics.h>
#include <kernel/screen/lib/print.h>
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/log.h>
#include <kernel/screen/bootscreen/boot.h>

// CPU
#include <kernel/arch/x86_64/cpu.h>
#include <kernel/arch/x86_64/gdt/gdt.h>
#include <kernel/arch/x86_64/idt/idt.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <kernel/arch/x86_64/exceptions/timer.h>
#include <kernel/arch/x86_64/exceptions/panic.h>
#include <kernel/pci/pci.h>

// Memory
#include <kernel/mem/meminclude.h>

// Processes
#include <kernel/proc/process.h>
#include <kernel/proc/scheduler.h>

// modules
#include <kernel/devices/device_init.h>

void _start(void)
{
    // doccrOS start
    // Ensure that Limine base revision is supported and that we have a framebuffer
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Initialize framebuffer graphics
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    graphics_init(fb);
    draw_logo();

    bs.Clear(BS1);

    char buf[256]; //for all string operations

    {
        // Initialize mem
        physmem_init(memmap_request.response, hhdm_request.response);
        paging_init(hhdm_request.response);
        kheap_init();
    }

    draw_logo();

    {
        // Initialize the CPU
        cpu_detect();
        log("[CPU]", "Detected CPU\n");
        gdt_init();
        idt_init();
        timer_init(1000);
        timer_set_boot_time(); //for uptime command
    }

    pci_init();
    process_init();
    sched_init();
    devices_init();

    //should not reach here
    #if USE_HCF == 1
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
}

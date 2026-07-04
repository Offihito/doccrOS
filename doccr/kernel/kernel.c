/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: kernel.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "kernel.h"
#include "screen/lib/print.h"

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
#include <kernel/arch/hal/cpu.h>
#include <kernel/arch/hal/interrupts.h>
#include <kernel/arch/hal/timer.h>
#include <kernel/pci/pci.h>

// Memory
#include <kernel/mem/meminclude.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/tests/vmm/vmm_test.h>

// Processes
#include <kernel/proc/process.h>
#include <kernel/proc/scheduler.h>

// modules
#include <kernel/devices/device_init.h>
#include <kernel/devices/init.h>

// File system
#include <kernel/fs/vfs/vfs.h>

void _start(void)
{
    serial_init();
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Initialize framebuffer graphics
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    graphics_init(fb);
    draw_logo();
    bs.Clear(BS1);

    // backspace and tab have special texture, ik its a bad solution sowey :/
    print("\b-\t-\b-\t-\b-\t-\b-\t-\b-\t-\b-\t-\b \n", blue());
    print("|   ", blue());
    print("Welcome to doccrOS  ", white());
    print("| \n", blue());
    print("\b-\t-\b-\t-\b-\t-\b-\t-\b-\t-\b-\t-\b \n\n", blue());

    char buf[256]; //for all string operations

    {
        physmem_init(memmap_request.response, hhdm_request.response);
        paging_init(hhdm_request.response);
        kheap_init();
        vmm_init();
    }

    draw_logo();

    {
        // Initialize the CPU
        cpu_detect();
        log("[CPU]", "Detected CPU\n");
        cpu_early_init();
        timer_init(1000);
        timer_set_boot_time(); //for uptime command
    }

    pci_init();
    process_init();
    sched_init();

    vfs_init();
    rootfs_init();
    vfs_dump();

    devices_init();
    kernel_devices_init();
    //vmm_run_all_tests();

    //should not reach here
    #if USE_HCF == 1
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
}

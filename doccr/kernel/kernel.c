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
#include <kernel/arch/hal/panic.h>
#include <kernel/pci/pci.h>

// Memory
#include <kernel/mem/meminclude.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/tests/vmm/vmm_test.h>

// Processes
#include <kernel/proc/process.h>
#include <kernel/proc/thread.h>
#include <kernel/proc/demo_threads.h>
#include <kernel/proc/scheduler.h>

// modules
#include <kernel/devices/device_init.h>
#include <kernel/devices/init.h>

// File system
#include <kernel/fs/vfs/vfs.h>

// User
#include <kernel/user/init.h>

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

    //full buffer clear even tho they werent used yet
    bs.Clear(BS1);
    bs.Clear(BS2);
    bs.Clear(BS3);
    bs.Clear(BS4);

    print("Welcome to doccrOS \n\n", white());

    {
        physmem_init(memmap_request.response, hhdm_request.response);
        paging_init(hhdm_request.response);
        kheap_init();
        vmm_init();
    }

    bootscreen_bs3_init_backbuffer();

    draw_logo();

    {
        cpu_detect();
        log("[CPU]", "Detected CPU\n");
        cpu_early_init();
        vmm_cow_install_handler();
        timer_init(1000);
        timer_set_boot_time(); //for uptime command
    }

    pci_init();
    process_init();
    sched_init();

    proc_t *kproc = process_create("kernel");
    if (!kproc) panic("could not create kernel proc, rip");

    thread_t *t_idle = thread_create(kproc, "idle", idle_fn, NULL);
    thread_create(kproc, "worker-1",     worker_fn,     (void*)(u64)1);
    thread_create(kproc, "worker-2",     worker_fn,     (void*)(u64)2);
    thread_create(kproc, "worker-3",     worker_fn,     (void*)(u64)3);

    //g_debug_canary_thread = t_idle;


    sched_enable();

    vfs_init();
    rootfs_init();
    vfs_dump();

    devices_init();
    kernel_devices_init();

    user_start();

    //should not reach here
    #if USE_HCF == 1
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif
}

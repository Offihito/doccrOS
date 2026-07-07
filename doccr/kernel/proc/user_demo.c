/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: user_demo.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "user_demo.h"
#include "process.h"
#include "thread.h"
#include <kernel/mem/vmm/vmm.h>
#include <kernel/mem/meminclude.h>
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/log.h>
#include <kernel/arch/x86_64/user/syscall.h>
#include <kernel/communication/serial.h>

thread_t *g_debug_canary_thread = NULL;

#define CHECK_CANARY(label) \
    do { \
        if (g_debug_canary_thread) \
            printf("[CANARY] %s: 0x%llx\n", label, *(u64*)g_debug_canary_thread->rsp); \
    } while (0)

#define USER_CODE_VA  0x0000700000000000ULL
#define USER_STACK_VA 0x0000700000100000ULL
#define STR_OFFSET    128

static void build_user_demo_code(u8 *buf, u64 code_va)
{
    const char *msg = "Hello from userspace!\n";
    u64 msg_len = (u64)str_len(msg);
    u64 str_va  = code_va + STR_OFFSET;
    u64 off = 0;

    buf[off++] = 0x48; buf[off++] = 0xBF;              memcpy(buf + off, &str_va, 8); off += 8;

    buf[off++] = 0x48; buf[off++] = 0xBE;              memcpy(buf + off, &msg_len, 8); off += 8;

    buf[off++] = 0xB8;                                 u32 sw = SYS_WRITE;    memcpy(buf + off, &sw, 4); off += 4;

    buf[off++] = 0xCD; buf[off++] = 0x80;              // int 0x80

    buf[off++] = 0xB8;

    u32 se = SYS_EXIT;                                 memcpy(buf + off, &se, 4); off += 4;

    buf[off++] = 0xCD; buf[off++] = 0x80;              memcpy(buf + STR_OFFSET, msg, msg_len + 1);
}

void user_demo_init(void)
{
    CHECK_CANARY("start");

    proc_t *p = process_create_user("user_demo");
    CHECK_CANARY("after process_create_user");

    if (!p)
    {
        log("[USER]", "could not create demo proc\n", warning);
        return;
    }

    u64 code_va = vmm_space_alloc(
        p->space, USER_CODE_VA, 1,
        VMM_REGION_USER | VMM_REGION_READ | VMM_REGION_WRITE | VMM_REGION_EXEC
    );

    CHECK_CANARY("after code vmm_space_alloc");

    if (!code_va)
    {
        printf("[USER] code alloc failed\n");
        return;
    }

    u64 stack_va = vmm_space_alloc(
        p->space, USER_STACK_VA, 1,
        VMM_REGION_USER | VMM_REGION_READ | VMM_REGION_WRITE
    );

    CHECK_CANARY("after stack vmm_space_alloc");
    if (!stack_va)
    {
        printf(
            "[USER] stack alloc failed, physmem_free=%llu kheap_free=%llu\n",
            physmem_free_get(),
            kheap_get_free_size()
        );
        return;
    }

    u64 hhdm     = paging_get_hhdm_offset();
    u64 code_phys    = vmm_space_get_phys(p->space, code_va);
    u8 *code_ptr     = (u8 *)(code_phys + hhdm);

    memset(code_ptr, 0x90, PAGE_SIZE);
    CHECK_CANARY("after memset code page");

    build_user_demo_code(code_ptr, code_va);

    CHECK_CANARY("after build_user_demo_code");

    u64 user_stack_top = stack_va + PAGE_SIZE;
    thread_create_user(p, "user_demo_t", (thread_entry_t)code_va, NULL, user_stack_top);
    CHECK_CANARY("after thread_create_user");

    log("[USER]", "user_demo process created\n");
}

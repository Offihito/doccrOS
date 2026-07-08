/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: elf.c
 * CREATED BY: Offihito
 * MODIFIED BY: emex
 *
 */

#include "elf.h"

#include <kernel/proc/process.h>
#include <kernel/proc/thread.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/mem/meminclude.h>
#include <kernel/mem/paging/paging.h>
#include <kernel/communication/serial.h>
#include <kernel/screen/lib/log.h>

static inline u64 align_up(u64 v, u64 a)
{
    return (v + a - 1) & ~(a - 1);
}

static inline u64 align_down(u64 v, u64 a)
{
    return v & ~(a - 1);
}

static int elf_check(const u8 *data, u64 size)
{
    if (size < sizeof(elf64_ehdr_t))
        return -1;

    const elf64_ehdr_t *eh = (const elf64_ehdr_t *)data;

    if (eh->e_ident[0] != ELF_MAGIC0 ||
        eh->e_ident[1] != ELF_MAGIC1 ||
        eh->e_ident[2] != ELF_MAGIC2 ||
        eh->e_ident[3] != ELF_MAGIC3)
    {
        printf("[ELF] bad magic\n");
        return -1;
    }

    if (eh->e_ident[4] != ELF_CLASS64)
    {
        printf("[ELF] not ELF64\n");
        return -1;
    }
    if (eh->e_ident[5] != ELF_DATA2LSB)
    {
        printf("[ELF] not little-endian\n");
        return -1;
    }
    if (eh->e_type != ELF_TYPE_EXEC && eh->e_type != ELF_TYPE_DYN)
    {
        printf("[ELF] not executable (e_type=%u)\n", (u32)eh->e_type);
        return -1;
    }
    if (eh->e_machine != ELF_MACH_X86_64)
    {
        printf("[ELF] not x86-64\n");
        return -1;
    }
    if (eh->e_phentsize < sizeof(elf64_phdr_t))
    {
        printf("[ELF] phentsize too small\n");
        return -1;
    }

    u64 ph_end = eh->e_phoff + (u64)eh->e_phnum * eh->e_phentsize;
    if (ph_end > size)
    {
        printf("[ELF] phdrs out of bounds\n");
        return -1;
    }

    return 0;
}

#define USER_STACK_PAGES 16ULL
#define USER_STACK_TOP   0x00007FFFFFFFE000ULL
#define USER_STACK_BASE  (USER_STACK_TOP - USER_STACK_PAGES * 4096ULL)

int elf_load(const u8 *data, u64 size, const char *name)
{
    if (!data || size == 0)
    {
        printf("[ELF] load '%s': null data or zero size\n", name ? name : "?");
        return -1;
    }

    if (elf_check(data, size) != 0)
        return -1;

    const elf64_ehdr_t *eh = (const elf64_ehdr_t *)data;
    u64 hhdm = paging_get_hhdm_offset();

    printf(
        "[ELF] loading '%s': type=%u machine=0x%x entry=0x%llx phdrs=%u\n",
        name,
        (u32)eh->e_type,
        (u32)eh->e_machine,
        eh->e_entry,
        (u32)eh->e_phnum
    );

    proc_t *p = process_create_user(name);
    if (!p)
    {
        printf("[ELF] process_create_user failed\n");
        return -1;
    }

    for (u16 i = 0; i < eh->e_phnum; i++)
    {
        const elf64_phdr_t *ph =
            (const elf64_phdr_t *)(data + eh->e_phoff + (u64)i * eh->e_phentsize);

        if (ph->p_type != PT_LOAD) continue;

        printf(
            "[ELF] seg %u: vaddr=0x%llx filesz=%llu memsz=%llu flags=%c%c%c\n",
            (u32)i,
            ph->p_vaddr,
            ph->p_filesz,
            ph->p_memsz,
            (ph->p_flags & PF_R) ? 'R' : '-',
            (ph->p_flags & PF_W) ? 'W' : '-',
            (ph->p_flags & PF_X) ? 'X' : '-'
        );

        if (ph->p_offset + ph->p_filesz > size)
        {
            printf(
                "[ELF] seg %u out of bounds (off=%llu filesz=%llu total=%llu)\n",
                (u32)i,
                ph->p_offset,
                ph->p_filesz,
                size
            );

            process_destroy(p);
            return -1;
        }

        if (ph->p_memsz < ph->p_filesz)
        {
            printf("[ELF] seg %u memsz < filesz\n", (u32)i);
            process_destroy(p);
            return -1;
        }

        u64 va_base      = align_down(ph->p_vaddr, 4096);
        u64 va_end       = align_up(ph->p_vaddr + ph->p_memsz, 4096);
        u64 pg_count     = (va_end - va_base) / 4096;

        u32 vmm_flags = VMM_REGION_USER | VMM_REGION_READ;

        if (ph->p_flags & PF_W) vmm_flags |= VMM_REGION_WRITE;
        if (ph->p_flags & PF_X) vmm_flags |= VMM_REGION_EXEC;

        u64 mapped = vmm_space_alloc(p->space, va_base, pg_count, vmm_flags);
        if (!mapped)
        {
            printf(
                "[ELF] seg %u vmm_space_alloc failed (va=0x%llx pages=%llu)\n",
                (u32)i,
                va_base,
                pg_count
            );
            process_destroy(p);
            return -1;
        }

        u64 file_src      = (u64)(data + ph->p_offset);
        u64 file_rem      = ph->p_filesz;
        u64 va_cursor     = ph->p_vaddr;
        u64 mem_rem       = ph->p_memsz;

        while (mem_rem > 0)
        {
            u64 page_va      = align_down(va_cursor, 4096);
            u64 page_off     = va_cursor - page_va;
            u64 phys         = vmm_space_get_phys(p->space, page_va);

            if (!phys)
            {
                printf("[ELF] seg %u get_phys failed va=0x%llx\n", (u32)i, page_va);
                process_destroy(p);
                return -1;
            }

            u8 *dest      = (u8 *)(phys + hhdm + page_off);
            u64 chunk     = 4096 - page_off;

            if (chunk > mem_rem) chunk = mem_rem;

            if (file_rem > 0)
            {
                u64 copy =      (chunk < file_rem) ? chunk : file_rem;
                memcpy(dest, (const void *)file_src, copy);
                file_src     += copy;
                file_rem     -= copy;
                dest         += copy;
                chunk        -= copy;
            }

            if (chunk > 0) memset(dest, 0, chunk);

            u64 step = 4096 - page_off;
            if (step > mem_rem) step = mem_rem;
            va_cursor += step;
            mem_rem   -= step;
        }

        printf("[ELF] seg %u mapped va=0x%llx pages=%llu\n", (u32)i, va_base, pg_count);
    }

    u64 stack = vmm_space_alloc(
        p->space,
        USER_STACK_BASE,
        USER_STACK_PAGES,
        VMM_REGION_USER | VMM_REGION_READ | VMM_REGION_WRITE
    );

    if (!stack)
    {
        printf(
            "[ELF] stack alloc failed (base=0x%llx pages=%llu)\n",
            USER_STACK_BASE,
            USER_STACK_PAGES
        );

        process_destroy(p);
        return -1;
    }

    printf(
        "[ELF] stack mapped: 0x%llx - 0x%llx\n",
        USER_STACK_BASE,
        USER_STACK_TOP
    );

    thread_t *t = thread_create_user(
        p,
        name,
        (thread_entry_t)eh->e_entry,
        NULL,
        USER_STACK_TOP
    );

    if (!t)
    {
        printf("[ELF] thread_create_user failed\n");
        process_destroy(p);
        return -1;
    }

    printf(
        "[ELF] launched '%s' entry=0x%llx stack_top=0x%llx\n",
        name,
        eh->e_entry,
        USER_STACK_TOP
    );

    log("[ELF]", "process scheduled\n", success);
    return 0;
}

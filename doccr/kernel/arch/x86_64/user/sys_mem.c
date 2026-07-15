/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys_mem.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "sys_mem.h"
#include <kernel/proc/process.h>
#include <kernel/mem/vmm/vmm.h>
#include <kernel/communication/serial.h>

// how big a page is, just in case someone forgot
#define PAGE_SIZE 4096


#define MMAP_PROT_READ     0x1
#define MMAP_PROT_WRITE    0x2
#define MMAP_PROT_EXEC     0x4
#define MMAP_MAP_FIXED   0x10
#define MMAP_MAP_ANON    0x20

// check if a virtual address range is completely unoccupied in this space
static int is_vaddr_range_free(vmm_space_t *space, u64 vaddr, u64 size)
{
    if (vaddr < 0x1000000ULL || vaddr + size > 0x00007FFFFFFFFFFFULL)
    {
        return  0;
    }

    vmm_region_t *cur  = space->regions;
    while (cur)
    {
        u64 cur_end   = cur->base + cur->size;
        u64 candidate_end = vaddr + size;

        // check if they overlap
        if (!(cur_end <= vaddr || cur->base >= candidate_end))
        {
            return 0; // overlap found!
        }
        cur     = cur->next;
    }
    return 1; // free
}
static u64 find_free_vaddr(vmm_space_t *space, u64 size)
{
    u64 vaddr_start = 0x50000000ULL;
    vmm_region_t *cur = space->regions;

    while (cur)
    {
        if (cur->base + cur->size <= vaddr_start)
        {
            cur = cur->next;
            continue;
        }

        if (cur->base >= vaddr_start + size)
        {
            break; // found a gap
        }

        vaddr_start = cur->base + cur->size;
        cur     = cur->next;
    }

    if (vaddr_start + size > 0x00007FFFFFFFFFFFULL)
    {
        return  0;
    }

    return vaddr_start;
}


void sys_mmap(cpu_state_t *state)
{
    u64 hint    = state->rdi;
    u64 length  = state->rsi;
    u64 prot    = state->rdx;
    u64 flags   = state->r10;

    // not rn mate
    if (!(flags & MMAP_MAP_ANON))
    {
        state->rax = (u64)-1; // MAP_ANONYMOUS or bust
        return;
    }

    if (length  == 0)
    {
        state->rax = (u64)-1; // mapping zero bytes is dumb
        return;
    }

    proc_t *p   = process_get_current();
    if (!p ||   !p->space)
    {
        state->rax = (u64)-1;
        return;
    }

    // round length up to page boundary
    u64 page_count = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    u64 size    = page_count * PAGE_SIZE;

    // build vmm flags from the prot bits
    u32 vmm_flags  = VMM_REGION_USER  | VMM_REGION_USED;
    if (prot &  MMAP_PROT_READ)  vmm_flags |= VMM_REGION_READ;
    if (prot &  MMAP_PROT_WRITE) vmm_flags |= VMM_REGION_WRITE;
    if (prot &  MMAP_PROT_EXEC)  vmm_flags |= VMM_REGION_EXEC;

    u64 target_vaddr = 0;

    if (hint != 0)
    {
        u64 aligned_hint = hint & ~(u64)(PAGE_SIZE - 1);
        if (flags & MMAP_MAP_FIXED)
        {
            if (!is_vaddr_range_free(p->space, aligned_hint, size))
            {
                vmm_space_free(p->space, aligned_hint);
            }
            target_vaddr     = aligned_hint;
        }
        else if (is_vaddr_range_free(p->space, aligned_hint, size))
        {
            target_vaddr     = aligned_hint;
        }
    }

    if (target_vaddr == 0)
    {
        target_vaddr = find_free_vaddr(p->space, size);
        if (target_vaddr == 0)
        {
            state->rax = (u64)-1;
            return;
        }
    }

    u64 vaddr   = vmm_space_alloc(p->space, target_vaddr, page_count, vmm_flags);
    if (vaddr   == 0)
    {
        state->rax = (u64)-1; // mapping failed
        return;
    }

    state->rax  = vaddr;
}
void sys_munmap(cpu_state_t *state)
{
    u64 addr    = state->rdi;

    // userspace addresses only
    if (addr    == 0 || addr > 0x00007FFFFFFFFFFFULL)
    {
        state->rax = (u64)-1;
        return;
    }

    proc_t *p   = process_get_current();
    if (!p || !p->space)
    {
        state->rax = (u64)-1;
        return;
    }

    // first we need to check if it exists
    vmm_region_t *region = vmm_space_find(p->space, addr);
    if (!region)
    {
        state->rax = (u64)-1; // nothing to unmap here, bye go home

        return;
    }

    vmm_space_free(p->space, addr);
    state->rax  = 0;
}
void sys_brk(cpu_state_t *state)
{
    u64 new_break = state->rdi;

    proc_t *p   = process_get_current();
    if (!p)
    {
        state->rax = (u64)-1;
        return;
    }

    if (p->heap_break == 0)
    {
        // sensible start
        // low in userspace
        p->heap_break = 0x0000000010000000ULL;
    }

    // askes 0, return current
    if (new_break == 0)
    {
        state->rax = p->heap_break;
        return;
    }

    // reject any obv garbage addresses
    if (new_break > 0x00007FFFFFFFFFFFULL)
    {
        state->rax = (u64)-1 ;
        return;
    }

    if (new_break <= p->heap_break)
    {
        // simple shrinking not unmap
        p->heap_break = new_break;
        state->rax    = new_break;
        return;
    }

    //then grow heap
    u64 old_break  = p->heap_break;
    u64 grow_bytes = new_break - old_break;
    u64 pages      = (grow_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

    u32 vmm_flags =
    	VMM_REGION_USER  |
        VMM_REGION_USED  |
        VMM_REGION_READ  |
        VMM_REGION_WRITE
    ;

    u64 mapped  = vmm_space_alloc(p->space, old_break, pages, vmm_flags);
    if (mapped  == 0)
    {
        //if not
        // return old break
        state->rax = old_break;
        return;
    }

    p->heap_break = new_break;
    state->rax  = new_break;
}

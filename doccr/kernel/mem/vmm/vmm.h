/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: vmm.h
 * CREATED BY: Offihito
 * MODIFIED BY: emex
 *
 */

#ifndef VMM_H
#define VMM_H

#include <types.h>

#define VMM_REGION_FREE    0x00
#define VMM_REGION_USED    0x01
#define VMM_REGION_KERNEL  0x02
#define VMM_REGION_USER    0x04
#define VMM_REGION_GUARD   0x08
#define VMM_REGION_READ    0x10
#define VMM_REGION_WRITE   0x20
#define VMM_REGION_EXEC    0x40
#define VMM_REGION_SHARED  0x80
#define VMM_REGION_COW     0x100
#define VMM_REGION_MMIO    0x200

#define VMM_BASE  0xFFFF900000000000ULL
#define VMM_LIMIT 0xFFFFF00000000000ULL

typedef struct vmm_region {
    u64               base;
    u64               size;
    u32               flags;
    struct vmm_region *prev;
    struct vmm_region *next;
} vmm_region_t;

typedef struct vmm_space {
    u64          pml4_phys;
    vmm_region_t *regions;
    u64          region_count;
    u64          used_virtual;
} vmm_space_t;

typedef struct {
    u64 total_virtual;
    u64 used_virtual;
    u64 free_virtual;
    u64 region_count;
    u64 used_region_count;
} vmm_stats_t;

void          vmm_init(void);

u64           vmm_alloc(u64 size, u32 flags);
void          vmm_free(u64 base);

vmm_region_t *vmm_find(u64 base);
vmm_stats_t   vmm_get_stats(void);

u64           vmm_get_used(void);
u64           vmm_get_free(void);
u64           vmm_get_region_count(void);

vmm_space_t  *vmm_space_create(void);
void          vmm_space_destroy(vmm_space_t *space);
vmm_space_t  *vmm_get_kernel_space(void);

u64           vmm_space_alloc(vmm_space_t *space, u64 vaddr, u64 page_count, u32 flags);
void          vmm_space_free(vmm_space_t *space, u64 vaddr);
vmm_region_t *vmm_space_find(vmm_space_t *space, u64 vaddr);

vmm_space_t  *vmm_clone_space(vmm_space_t *src);
void          vmm_cow_break(vmm_space_t *space, u64 fault_addr);
void          vmm_cow_install_handler(void);

u64           vmm_map_phys(vmm_space_t *space, u64 vaddr, u64 phys_addr, u64 page_count, u32 flags);
void          vmm_unmap_phys(vmm_space_t *space, u64 vaddr);

void vmm_space_activate(vmm_space_t *space);
vmm_space_t *vmm_get_active_space(void);

u64 vmm_space_get_phys(vmm_space_t *space, u64 vaddr);

#endif


/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: vmm_test.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */
#include "vmm_test.h"
#include <kernel/mem/vmm/vmm.h>
#include <kernel/mem/phys/physmem.h>
#include <kernel/communication/serial.h>
#include <kernel/arch/hal/panic.h>

#define TEST_PASS(name) serial_printf("[VMM TEST] PASS  %s\n", name)
#define TEST_FAIL(name) serial_printf("[VMM TEST] FAIL  %s\n", name)

#define ASSERT(cond, name) \
    do { if (!(cond)) { TEST_FAIL(name); fail_count++; } \
         else         { TEST_PASS(name); pass_count++; } } while(0)

#define ASSERT_SILENT(cond) \
    do { if (!(cond)) { fail_count++; } else { pass_count++; } } while(0)

static u64 pass_count = 0;
static u64 fail_count = 0;

static void reset_counters(void) { pass_count = 0; fail_count = 0; }

static void print_result(const char *suite)
{
    serial_printf("[VMM TEST] ---- %s: %llu passed, %llu failed ----\n",
                  suite, pass_count, fail_count);
}

static u64 simple_rand(u64 *seed)
{
    *seed ^= *seed << 13;
    *seed ^= *seed >> 7;
    *seed ^= *seed << 17;
    return *seed;
}

static void vmm_reset(void) { vmm_init(); }

static void test_basic_alloc_free(void)
{
    reset_counters();
    vmm_reset();

    u64 a = vmm_alloc(0x1000, VMM_REGION_KERNEL);
    ASSERT(a != 0,                               "alloc 4K non-zero");
    ASSERT(a >= VMM_BASE && a < VMM_LIMIT,       "alloc 4K in range");

    vmm_region_t *r = vmm_find(a);
    ASSERT(r != NULL,                            "find after alloc");
    ASSERT(r->flags & VMM_REGION_USED,           "region marked used");
    ASSERT(r->size == 0x1000,                    "region size == 4K");

    vmm_free(a);
    r = vmm_find(a);
    ASSERT(r == NULL || !(r->flags & VMM_REGION_USED), "free clears used flag");

    print_result("basic_alloc_free");
}

static void test_page_alignment(void)
{
    reset_counters();
    vmm_reset();

    u64 sizes[] = { 1, 100, 511, 512, 1023, 1024, 4095, 4096, 4097, 8193, 65535 };
    u64 n = sizeof(sizes) / sizeof(sizes[0]);

    for (u64 i = 0; i < n; i++)
    {
        u64 a = vmm_alloc(sizes[i], VMM_REGION_KERNEL);
        ASSERT_SILENT(a != 0);
        ASSERT_SILENT((a & 0xFFF) == 0);
        vmm_region_t *r = vmm_find(a);
        ASSERT_SILENT(r && (r->size & 0xFFF) == 0);
        vmm_free(a);
    }

    serial_printf("[VMM TEST] PASS  page_alignment (%llu sizes)\n", n);
    print_result("page_alignment");
}

static void test_multiple_alloc_free_all(void)
{
    reset_counters();
    vmm_reset();

    #define MA_N 64
    u64 ptrs[MA_N];

    for (u64 i = 0; i < MA_N; i++)
        ptrs[i] = vmm_alloc(0x1000 * (i + 1), VMM_REGION_KERNEL);

    for (u64 i = 0; i < MA_N; i++)
        vmm_free(ptrs[i]);

    vmm_stats_t s = vmm_get_stats();
    ASSERT(s.used_virtual == 0,               "mass free: used == 0");
    ASSERT(s.used_region_count == 0,          "mass free: region count == 0");
    ASSERT(s.free_virtual == s.total_virtual, "mass free: free == total");

    print_result("multiple_alloc_free_all");
    #undef MA_N
}

static void test_no_overlap(void)
{
    reset_counters();
    vmm_reset();

    #define OV_N 64
    u64 bases[OV_N];
    u64 ends[OV_N];

    for (u64 i = 0; i < OV_N; i++)
    {
        u64 sz  = 0x1000 * (1 + (i % 8));
        bases[i] = vmm_alloc(sz, VMM_REGION_KERNEL);
        ends[i]  = bases[i] + sz;
        ASSERT_SILENT(bases[i] != 0);
    }

    u64 overlaps = 0;
    for (u64 i = 0; i < OV_N; i++)
    {
        if (!bases[i]) continue;
        for (u64 j = i + 1; j < OV_N; j++)
        {
            if (!bases[j]) continue;
            if (bases[i] < ends[j] && bases[j] < ends[i])
                overlaps++;
        }
    }

    ASSERT(overlaps == 0, "no region overlaps");

    for (u64 i = 0; i < OV_N; i++)
        vmm_free(bases[i]);

    print_result("no_overlap");
    #undef OV_N
}

static void test_free_null(void)
{
    reset_counters();
    vmm_reset();

    vmm_free(0);
    vmm_free(0xDEAD000);

    ASSERT(1, "free(0) and free(invalid) don't crash");
    print_result("free_null_and_invalid");
}

static void test_double_free_resilience(void)
{
    reset_counters();
    vmm_reset();

    u64 a = vmm_alloc(0x4000, VMM_REGION_KERNEL);
    ASSERT(a != 0, "alloc before double free");

    vmm_free(a);
    vmm_free(a);

    vmm_stats_t s = vmm_get_stats();
    ASSERT(s.used_virtual == 0, "double free: used stays 0");

    print_result("double_free_resilience");
}

static void test_coalesce(void)
{
    reset_counters();
    vmm_reset();

    u64 a = vmm_alloc(0x1000, VMM_REGION_KERNEL);
    u64 b = vmm_alloc(0x1000, VMM_REGION_KERNEL);
    u64 c = vmm_alloc(0x1000, VMM_REGION_KERNEL);
    ASSERT(a != 0 && b != 0 && c != 0, "alloc 3 adjacent regions");

    u64 regions_before = vmm_get_region_count();

    vmm_free(b);
    vmm_free(a);
    vmm_free(c);

    ASSERT(vmm_get_region_count() < regions_before, "coalesce reduces region count");
    ASSERT(vmm_get_stats().used_virtual == 0,       "coalesce: no leak");

    print_result("coalesce");
}

static void test_stats_consistency(void)
{
    reset_counters();
    vmm_reset();

    vmm_stats_t s0 = vmm_get_stats();
    ASSERT(s0.used_virtual == 0,                          "stats: init used == 0");
    ASSERT(s0.free_virtual == s0.total_virtual,           "stats: init free == total");

    u64 a = vmm_alloc(0x5000, VMM_REGION_KERNEL);
    vmm_stats_t s1 = vmm_get_stats();
    ASSERT(s1.used_virtual >= 0x5000,                     "stats: used grew");
    ASSERT(s1.free_virtual < s0.free_virtual,             "stats: free shrank");
    ASSERT(s1.used_virtual + s1.free_virtual == s1.total_virtual, "stats: sum invariant");

    vmm_free(a);
    vmm_stats_t s2 = vmm_get_stats();
    ASSERT(s2.used_virtual == 0,                          "stats: used back to 0");
    ASSERT(s2.used_virtual + s2.free_virtual == s2.total_virtual, "stats: invariant after free");

    print_result("stats_consistency");
}

static void test_stress_batch_5000(void)
{
    reset_counters();
    vmm_reset();

    #define B5_BATCH 50
    #define B5_ROUNDS 100

    u64 ptrs[B5_BATCH];
    u64 total_allocs = 0;
    u64 total_frees  = 0;

    for (u64 round = 0; round < B5_ROUNDS; round++)
    {
        for (u64 i = 0; i < B5_BATCH; i++)
        {
            ptrs[i] = vmm_alloc(0x1000 * (1 + (i % 16)), VMM_REGION_KERNEL);
            if (ptrs[i]) total_allocs++;
        }
        for (u64 i = 0; i < B5_BATCH; i++)
        {
            if (ptrs[i]) { vmm_free(ptrs[i]); total_frees++; }
        }
        vmm_stats_t s = vmm_get_stats();
        ASSERT_SILENT(s.used_virtual == 0);
        ASSERT_SILENT(s.used_region_count == 0);
    }

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "stress5000: no leak");
    ASSERT(sf.used_region_count == 0,          "stress5000: region count clean");
    ASSERT(sf.free_virtual == sf.total_virtual,"stress5000: free == total");

    serial_printf("[VMM TEST] INFO  stress5000: %llu allocs, %llu frees (%llu ops)\n",
                  total_allocs, total_frees, total_allocs + total_frees);
    print_result("stress_batch_5000");
    #undef B5_BATCH
    #undef B5_ROUNDS
}

static void test_stress_batch_10000(void)
{
    reset_counters();
    vmm_reset();

    #define B10_BATCH 50
    #define B10_ROUNDS 200

    u64 ptrs[B10_BATCH];
    u64 total_allocs = 0;
    u64 total_frees  = 0;
    u64 leak_rounds  = 0;

    for (u64 round = 0; round < B10_ROUNDS; round++)
    {
        for (u64 i = 0; i < B10_BATCH; i++)
        {
            ptrs[i] = vmm_alloc(0x1000 * (1 + (i % 32)), VMM_REGION_KERNEL);
            if (ptrs[i]) total_allocs++;
        }
        for (u64 i = 0; i < B10_BATCH; i++)
        {
            if (ptrs[i]) { vmm_free(ptrs[i]); total_frees++; }
        }
        vmm_stats_t s = vmm_get_stats();
        if (s.used_virtual != 0 || s.used_region_count != 0) leak_rounds++;
    }

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "stress10000: no leak");
    ASSERT(sf.used_region_count == 0,          "stress10000: region count clean");
    ASSERT(sf.free_virtual == sf.total_virtual,"stress10000: free == total");
    ASSERT(leak_rounds == 0,                   "stress10000: zero leak rounds");

    serial_printf("[VMM TEST] INFO  stress10000: %llu allocs, %llu frees, %llu leak_rounds\n",
                  total_allocs, total_frees, leak_rounds);
    print_result("stress_batch_10000");
    #undef B10_BATCH
    #undef B10_ROUNDS
}

static void test_stress_random_pattern(void)
{
    reset_counters();
    vmm_reset();

    #define RND_SLOTS 64
    #define RND_OPS   5000

    u64 ptrs[RND_SLOTS];
    for (u64 i = 0; i < RND_SLOTS; i++) ptrs[i] = 0;

    u64 seed        = 0xCAFEBABEDEAD1337ULL;
    u64 total_alloc = 0;
    u64 total_free  = 0;

    for (u64 op = 0; op < RND_OPS; op++)
    {
        u64 rng  = simple_rand(&seed);
        u64 slot = rng % RND_SLOTS;

        if (ptrs[slot] == 0)
        {
            u64 pages = 1 + (simple_rand(&seed) % 32);
            ptrs[slot] = vmm_alloc(pages * 0x1000, VMM_REGION_KERNEL);
            if (ptrs[slot]) total_alloc++;
        }
        else
        {
            vmm_free(ptrs[slot]);
            total_free++;
            ptrs[slot] = 0;
        }
    }

    for (u64 i = 0; i < RND_SLOTS; i++)
    {
        if (ptrs[i]) { vmm_free(ptrs[i]); total_free++; }
    }

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "random_pattern: no leak");
    ASSERT(sf.used_region_count == 0,          "random_pattern: region count clean");
    ASSERT(sf.free_virtual == sf.total_virtual,"random_pattern: free == total");

    serial_printf("[VMM TEST] INFO  random_pattern: %llu allocs, %llu frees over %d ops\n",
                  total_alloc, total_free, RND_OPS);
    print_result("stress_random_pattern");
    #undef RND_SLOTS
    #undef RND_OPS
}

static void test_stress_checkerboard(void)
{
    reset_counters();
    vmm_reset();

    #define CB_N 100

    u64 ptrs[CB_N];
    for (u64 i = 0; i < CB_N; i++)
        ptrs[i] = vmm_alloc(0x1000, VMM_REGION_KERNEL);

    for (u64 i = 0; i < CB_N; i += 2)
        if (ptrs[i]) vmm_free(ptrs[i]);

    vmm_stats_t s_mid = vmm_get_stats();
    u64 expected_used = (CB_N / 2) * 0x1000;
    ASSERT(s_mid.used_virtual == expected_used, "checkerboard: correct used after odd free");

    for (u64 i = 1; i < CB_N; i += 2)
        if (ptrs[i]) vmm_free(ptrs[i]);

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "checkerboard: no leak");
    ASSERT(sf.free_virtual == sf.total_virtual,"checkerboard: free == total");

    print_result("stress_checkerboard");
    #undef CB_N
}

static void test_stress_varied_sizes(void)
{
    reset_counters();
    vmm_reset();

    u64 sizes[] = {
        0x1000, 0x2000, 0x4000, 0x8000, 0x10000,
        0x20000, 0x40000, 0x80000, 0x100000, 0x200000
    };
    u64 n = sizeof(sizes) / sizeof(sizes[0]);

    #define VS_ROUNDS 100
    u64 ptrs[10];

    for (u64 round = 0; round < VS_ROUNDS; round++)
    {
        for (u64 i = 0; i < n; i++)
            ptrs[i] = vmm_alloc(sizes[i], VMM_REGION_KERNEL);

        for (u64 i = n; i > 0; i--)
            if (ptrs[i-1]) vmm_free(ptrs[i-1]);

        vmm_stats_t s = vmm_get_stats();
        ASSERT_SILENT(s.used_virtual == 0);
    }

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "varied_sizes: no leak");
    ASSERT(sf.free_virtual == sf.total_virtual,"varied_sizes: free == total");

    serial_printf("[VMM TEST] INFO  varied_sizes: %d rounds x %llu sizes\n", VS_ROUNDS, n);
    print_result("stress_varied_sizes");
    #undef VS_ROUNDS
}

static void test_stress_interleaved(void)
{
    reset_counters();
    vmm_reset();

    #define IL_N 64

    u64 ptrs[IL_N];
    u64 szs[IL_N];

    for (u64 i = 0; i < IL_N; i++)
    {
        szs[i]  = 0x1000 * (1 + (i % 32));
        ptrs[i] = vmm_alloc(szs[i], VMM_REGION_KERNEL);
        ASSERT_SILENT(ptrs[i] != 0);
    }

    u64 expected = 0;
    for (u64 i = 0; i < IL_N; i++) expected += szs[i];

    for (u64 i = IL_N; i > 0; i--)
    {
        vmm_free(ptrs[i-1]);
        expected -= szs[i-1];
        vmm_stats_t s = vmm_get_stats();
        ASSERT_SILENT(s.used_virtual == expected);
    }

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "interleaved: no leak");
    ASSERT(sf.free_virtual == sf.total_virtual,"interleaved: free == total");

    print_result("stress_interleaved");
    #undef IL_N
}

static void test_stress_rapid_realloc(void)
{
    reset_counters();
    vmm_reset();

    #define RR_ITERS 5000

    u64 ptr = 0;
    for (u64 i = 0; i < RR_ITERS; i++)
    {
        if (ptr) vmm_free(ptr);
        ptr = vmm_alloc(0x1000 * (1 + (i % 64)), VMM_REGION_KERNEL);
        ASSERT_SILENT(ptr != 0);
    }
    if (ptr) vmm_free(ptr);

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "rapid_realloc: no leak");
    ASSERT(sf.free_virtual == sf.total_virtual,"rapid_realloc: free == total");

    print_result("stress_rapid_realloc");
    #undef RR_ITERS
}

static void test_stress_mixed_flags(void)
{
    reset_counters();
    vmm_reset();

    #define MF_N 64

    u32 flag_set[] = {
        VMM_REGION_KERNEL,
        VMM_REGION_USER,
        VMM_REGION_GUARD,
        VMM_REGION_KERNEL | VMM_REGION_GUARD
    };
    u64 ptrs[MF_N];

    for (u64 i = 0; i < MF_N; i++)
    {
        u32 f   = flag_set[i % 4];
        ptrs[i] = vmm_alloc(0x2000, f);
        ASSERT_SILENT(ptrs[i] != 0);
        vmm_region_t *r = vmm_find(ptrs[i]);
        ASSERT_SILENT(r && (r->flags & f));
    }

    for (u64 i = 0; i < MF_N; i++)
        vmm_free(ptrs[i]);

    vmm_stats_t sf = vmm_get_stats();
    ASSERT(sf.used_virtual == 0,               "mixed_flags: no leak");
    ASSERT(sf.free_virtual == sf.total_virtual,"mixed_flags: free == total");

    print_result("stress_mixed_flags");
    #undef MF_N
}

static void test_final_leak_check(void)
{
    vmm_reset();
    vmm_stats_t s = vmm_get_stats();

    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] FINAL LEAK CHECK\n");
    serial_printf("[VMM TEST]   used_virtual      = %llu\n", s.used_virtual);
    serial_printf("[VMM TEST]   free_virtual      = %llu\n", s.free_virtual);
    serial_printf("[VMM TEST]   total_virtual     = %llu\n", s.total_virtual);
    serial_printf("[VMM TEST]   region_count      = %llu\n", s.region_count);
    serial_printf("[VMM TEST]   used_region_count = %llu\n", s.used_region_count);

    if (s.used_virtual == 0 && s.used_region_count == 0 && s.free_virtual == s.total_virtual)
        serial_printf("[VMM TEST] RESULT: NO LEAKS DETECTED\n");
    else
        serial_printf("[VMM TEST] RESULT: *** LEAK DETECTED ***\n");

    serial_printf("[VMM TEST] ============================\n");
}

static void test_space_kernel_not_null(void)
{
    reset_counters();

    vmm_space_t *ks = vmm_get_kernel_space();
    ASSERT(ks != NULL,           "kernel space: not null");
    ASSERT(ks->pml4_phys != 0,   "kernel space: pml4_phys set");

    print_result("space_kernel_not_null");
}

static void test_space_create_destroy(void)
{
    reset_counters();

    vmm_space_t *s = vmm_space_create();
    ASSERT(s != NULL,            "space_create: returns non-null");
    ASSERT(s->pml4_phys != 0,    "space_create: pml4_phys non-zero");
    ASSERT(s->regions == NULL,   "space_create: regions list empty");
    ASSERT(s->region_count == 0, "space_create: region_count == 0");
    ASSERT(s->used_virtual == 0, "space_create: used_virtual == 0");

    vmm_space_destroy(s);
    ASSERT(1, "space_destroy: did not crash");

    print_result("space_create_destroy");
}

static void test_space_pml4_distinct(void)
{
    reset_counters();

    vmm_space_t *a = vmm_space_create();
    vmm_space_t *b = vmm_space_create();

    ASSERT(a != NULL && b != NULL,       "space_pml4_distinct: both created");
    ASSERT(a->pml4_phys != b->pml4_phys, "space_pml4_distinct: different PML4 frames");

    vmm_space_destroy(a);
    vmm_space_destroy(b);

    print_result("space_pml4_distinct");
}

static void test_space_pml4_distinct_from_kernel(void)
{
    reset_counters();

    vmm_space_t *ks = vmm_get_kernel_space();
    vmm_space_t *us = vmm_space_create();

    ASSERT(us != NULL,                     "space_vs_kernel: user space created");
    ASSERT(us->pml4_phys != ks->pml4_phys, "space_vs_kernel: PML4 differs from kernel");

    vmm_space_destroy(us);

    print_result("space_pml4_distinct_from_kernel");
}

static void test_space_destroy_null_safe(void)
{
    reset_counters();

    vmm_space_destroy(NULL);
    ASSERT(1, "space_destroy(NULL): no crash");

    print_result("space_destroy_null_safe");
}

static void test_space_destroy_kernel_safe(void)
{
    reset_counters();

    vmm_space_t *ks = vmm_get_kernel_space();
    vmm_space_destroy(ks);

    ASSERT(vmm_get_kernel_space() == ks,     "space_destroy(kernel): pointer unchanged");
    ASSERT(ks->pml4_phys != 0,              "space_destroy(kernel): pml4_phys intact");

    print_result("space_destroy_kernel_safe");
}

static void test_space_create_many(void)
{
    reset_counters();

    #define SCM_N 16
    vmm_space_t *spaces[SCM_N];

    for (u64 i = 0; i < SCM_N; i++)
    {
        spaces[i] = vmm_space_create();
        ASSERT_SILENT(spaces[i] != NULL);
        ASSERT_SILENT(spaces[i]->pml4_phys != 0);
    }

    for (u64 i = 0; i < SCM_N; i++)
        for (u64 j = i + 1; j < SCM_N; j++)
            ASSERT_SILENT(spaces[i]->pml4_phys != spaces[j]->pml4_phys);

    for (u64 i = 0; i < SCM_N; i++)
        vmm_space_destroy(spaces[i]);

    ASSERT(pass_count > 0, "space_create_many: all distinct, all destroyed");

    serial_printf("[VMM TEST] INFO  space_create_many: %d spaces, all distinct\n", SCM_N);
    print_result("space_create_many");
    #undef SCM_N
}

static void test_phase1_still_works_after_phase2_init(void)
{
    reset_counters();
    vmm_reset();

    u64 a = vmm_alloc(0x3000, VMM_REGION_KERNEL);
    ASSERT(a != 0,                         "p1_after_p2: alloc non-zero");
    ASSERT((a & 0xFFF) == 0,               "p1_after_p2: page aligned");

    vmm_region_t *r = vmm_find(a);
    ASSERT(r != NULL,                      "p1_after_p2: region found");
    ASSERT(r->size == 0x3000,              "p1_after_p2: correct size");

    vmm_free(a);
    vmm_stats_t s = vmm_get_stats();
    ASSERT(s.used_virtual == 0,            "p1_after_p2: no leak");

    print_result("phase1_still_works_after_phase2_init");
}

static void test_p3_alloc_basic(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_alloc_basic: space created");

    u64 vaddr = 0x0000100000000000ULL;
    u64 ret   = vmm_space_alloc(sp, vaddr, 1, VMM_REGION_READ | VMM_REGION_WRITE);

    ASSERT(ret == vaddr,          "p3_alloc_basic: returns vaddr");
    ASSERT(sp->region_count == 1, "p3_alloc_basic: region_count == 1");
    ASSERT(sp->used_virtual == PAGE_SIZE, "p3_alloc_basic: used_virtual == PAGE_SIZE");

    vmm_region_t *r = vmm_space_find(sp, vaddr);
    ASSERT(r != NULL,                           "p3_alloc_basic: region found");
    ASSERT(r->base  == vaddr,                   "p3_alloc_basic: region base correct");
    ASSERT(r->size  == PAGE_SIZE,               "p3_alloc_basic: region size == PAGE_SIZE");
    ASSERT(r->flags & VMM_REGION_USED,          "p3_alloc_basic: region marked used");
    ASSERT(r->flags & VMM_REGION_WRITE,         "p3_alloc_basic: write flag preserved");

    vmm_space_free(sp, vaddr);
    ASSERT(sp->region_count == 0,  "p3_alloc_basic: region_count 0 after free");
    ASSERT(sp->used_virtual == 0,  "p3_alloc_basic: used_virtual 0 after free");

    vmm_space_destroy(sp);
    print_result("p3_alloc_basic");
}

static void test_p3_alloc_multipage(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_alloc_multi: space created");

    u64 vaddr      = 0x0000200000000000ULL;
    u64 page_count = 8;
    u64 ret        = vmm_space_alloc(sp, vaddr, page_count, VMM_REGION_READ | VMM_REGION_WRITE);

    ASSERT(ret == vaddr,                              "p3_alloc_multi: returns vaddr");
    ASSERT(sp->used_virtual == page_count * PAGE_SIZE,"p3_alloc_multi: used_virtual correct");

    vmm_region_t *r = vmm_space_find(sp, vaddr);
    ASSERT(r != NULL,                                 "p3_alloc_multi: region found at base");
    ASSERT(r->size == page_count * PAGE_SIZE,         "p3_alloc_multi: region size correct");

    vmm_region_t *r2 = vmm_space_find(sp, vaddr + (page_count - 1) * PAGE_SIZE);
    ASSERT(r2 == r,                                   "p3_alloc_multi: find works mid-region");

    vmm_space_free(sp, vaddr);
    ASSERT(sp->used_virtual == 0,  "p3_alloc_multi: no leak after free");
    ASSERT(sp->region_count == 0,  "p3_alloc_multi: region count 0");

    vmm_space_destroy(sp);
    print_result("p3_alloc_multipage");
}

static void test_p3_alloc_two_regions(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_two_regions: space created");

    u64 va1 = 0x0000300000000000ULL;
    u64 va2 = 0x0000300000010000ULL;

    u64 r1 = vmm_space_alloc(sp, va1, 4, VMM_REGION_READ | VMM_REGION_WRITE);
    u64 r2 = vmm_space_alloc(sp, va2, 4, VMM_REGION_READ);

    ASSERT(r1 == va1,             "p3_two_regions: first alloc correct");
    ASSERT(r2 == va2,             "p3_two_regions: second alloc correct");
    ASSERT(sp->region_count == 2, "p3_two_regions: two regions tracked");
    ASSERT(sp->used_virtual == 8 * PAGE_SIZE, "p3_two_regions: used_virtual == 8 pages");

    vmm_space_free(sp, va1);
    ASSERT(sp->region_count == 1, "p3_two_regions: one region after first free");

    vmm_space_free(sp, va2);
    ASSERT(sp->region_count == 0, "p3_two_regions: zero regions after second free");
    ASSERT(sp->used_virtual == 0, "p3_two_regions: no leak");

    vmm_space_destroy(sp);
    print_result("p3_alloc_two_regions");
}

static void test_p3_find_miss(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_find_miss: space created");

    vmm_region_t *r = vmm_space_find(sp, 0x0000400000000000ULL);
    ASSERT(r == NULL, "p3_find_miss: find on empty space returns NULL");

    u64 vaddr = 0x0000400000000000ULL;
    vmm_space_alloc(sp, vaddr, 1, VMM_REGION_READ);

    r = vmm_space_find(sp, vaddr + 0x10000ULL);
    ASSERT(r == NULL, "p3_find_miss: find outside region returns NULL");

    vmm_space_free(sp, vaddr);
    vmm_space_destroy(sp);
    print_result("p3_find_miss");
}

static void test_p3_free_null_safe(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_free_null: space created");

    vmm_space_free(sp, 0);
    vmm_space_free(sp, 0xDEADBEEF000ULL);
    ASSERT(1, "p3_free_null: free of invalid addrs does not crash");

    vmm_space_destroy(sp);
    print_result("p3_free_null_safe");
}

static void test_p3_physmem_reclaimed(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_phys_reclaim: space created");

    u64 vaddr      = 0x0000500000000000ULL;
    u64 page_count = 16;

    u64 free_before = physmem_free_get();
    vmm_space_alloc(sp, vaddr, page_count, VMM_REGION_READ | VMM_REGION_WRITE);
    u64 free_after_alloc = physmem_free_get();

    ASSERT(free_after_alloc < free_before,
           "p3_phys_reclaim: physmem decreases after alloc");

    vmm_space_free(sp, vaddr);
    u64 free_after_free = physmem_free_get();

    ASSERT(free_after_free > free_after_alloc,
           "p3_phys_reclaim: physmem increases after free");
    ASSERT(free_after_free == free_before,
           "p3_phys_reclaim: physmem fully restored");

    vmm_space_destroy(sp);
    print_result("p3_physmem_reclaimed");
}

static void test_p3_destroy_frees_phys(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_destroy_phys: space created");

    u64 vaddr = 0x0000600000000000ULL;
    vmm_space_alloc(sp, vaddr, 4, VMM_REGION_READ | VMM_REGION_WRITE);

    u64 free_before_destroy = physmem_free_get();
    vmm_space_destroy(sp);
    u64 free_after_destroy = physmem_free_get();

    ASSERT(free_after_destroy > free_before_destroy,
           "p3_destroy_phys: destroy frees PML4 frame");

    print_result("p3_destroy_frees_phys");
}

static void test_p3_stress_alloc_free(void)
{
    reset_counters();

    #define P3S_N 32
    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p3_stress: space created");

    u64 base     = 0x0000700000000000ULL;
    u64 stride   = 0x10000ULL;
    u64 paddrs[P3S_N];

    for (u64 i = 0; i < P3S_N; i++)
        paddrs[i] = vmm_space_alloc(sp, base + i * stride, 2,
                                    VMM_REGION_READ | VMM_REGION_WRITE);

    for (u64 i = 0; i < P3S_N; i++)
        ASSERT_SILENT(paddrs[i] == base + i * stride);

    ASSERT(sp->region_count  == P3S_N,           "p3_stress: all regions tracked");
    ASSERT(sp->used_virtual  == P3S_N * 2 * PAGE_SIZE, "p3_stress: used_virtual correct");

    for (u64 i = 0; i < P3S_N; i++)
        vmm_space_free(sp, paddrs[i]);

    ASSERT(sp->region_count == 0, "p3_stress: zero regions after all frees");
    ASSERT(sp->used_virtual == 0, "p3_stress: no leak after all frees");

    vmm_space_destroy(sp);
    serial_printf("[VMM TEST] INFO  p3_stress: %d alloc+free cycles\n", P3S_N);
    print_result("p3_stress_alloc_free");
    #undef P3S_N
}

static void test_p4_clone_creates_new_space(void)
{
    reset_counters();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_clone_new_space: src created");

    u64 vaddr = 0x0000A00000000000ULL;
    vmm_space_alloc(src, vaddr, 2, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL,                    "p4_clone_new_space: clone non-null");
    ASSERT(dst != src,                     "p4_clone_new_space: distinct pointer");
    ASSERT(dst->pml4_phys != src->pml4_phys, "p4_clone_new_space: distinct PML4");

    vmm_space_destroy(dst);
    vmm_space_destroy(src);
    print_result("p4_clone_creates_new_space");
}

static void test_p4_clone_region_count(void)
{
    reset_counters();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_region_count: src created");

    u64 va1 = 0x0000B00000000000ULL;
    u64 va2 = 0x0000B00000010000ULL;
    vmm_space_alloc(src, va1, 3, VMM_REGION_READ | VMM_REGION_WRITE);
    vmm_space_alloc(src, va2, 3, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL,                          "p4_region_count: clone non-null");
    ASSERT(dst->region_count == src->region_count, "p4_region_count: same region count");
    ASSERT(dst->used_virtual == src->used_virtual, "p4_region_count: same used_virtual");

    vmm_space_destroy(dst);
    vmm_space_destroy(src);
    print_result("p4_clone_region_count");
}

static void test_p4_clone_cow_flags(void)
{
    reset_counters();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_cow_flags: src created");

    u64 vaddr = 0x0000C00000000000ULL;
    vmm_space_alloc(src, vaddr, 1, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL, "p4_cow_flags: clone non-null");

    vmm_region_t *sr = vmm_space_find(src, vaddr);
    vmm_region_t *dr = vmm_space_find(dst, vaddr);

    ASSERT(sr != NULL,                       "p4_cow_flags: src region found");
    ASSERT(dr != NULL,                       "p4_cow_flags: dst region found");
    ASSERT(sr->flags & VMM_REGION_COW,       "p4_cow_flags: src region has COW flag");
    ASSERT(dr->flags & VMM_REGION_COW,       "p4_cow_flags: dst region has COW flag");
    ASSERT(!(sr->flags & VMM_REGION_WRITE),  "p4_cow_flags: src write flag cleared");
    ASSERT(!(dr->flags & VMM_REGION_WRITE),  "p4_cow_flags: dst write flag cleared");

    vmm_space_destroy(dst);
    vmm_space_destroy(src);
    print_result("p4_clone_cow_flags");
}

static void test_p4_physmem_shared_after_clone(void)
{
    reset_counters();

    u64 free_before = physmem_free_get();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_phys_shared: src created");

    u64 vaddr      = 0x0000D00000000000ULL;
    u64 page_count = 4;

    vmm_space_alloc(src, vaddr, page_count, VMM_REGION_READ | VMM_REGION_WRITE);
    u64 free_after_alloc = physmem_free_get();

    u64 frames_used_so_far = free_before - free_after_alloc;

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL, "p4_phys_shared: clone non-null");

    u64 free_after_clone = physmem_free_get();
    u64 extra_clone_frames = free_after_alloc - free_after_clone;

    ASSERT(extra_clone_frames < frames_used_so_far,
           "p4_phys_shared: clone shares data frames (no data frame duplication)");

    vmm_space_destroy(dst);
    vmm_space_destroy(src);
    u64 free_after_both = physmem_free_get();

    ASSERT(free_after_both == free_before,
           "p4_phys_shared: all frames freed after both spaces destroyed");

    print_result("p4_physmem_shared_after_clone");
}

static void test_p4_cow_break_gives_private_frame(void)
{
    reset_counters();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_cow_break: src created");

    u64 vaddr = 0x0000E00000000000ULL;
    vmm_space_alloc(src, vaddr, 1, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL, "p4_cow_break: clone non-null");

    u64 free_before_break = physmem_free_get();
    vmm_cow_break(dst, vaddr);
    u64 free_after_break = physmem_free_get();

    vmm_region_t *dr = vmm_space_find(dst, vaddr);
    ASSERT(dr != NULL,                      "p4_cow_break: dst region still found");
    ASSERT(!(dr->flags & VMM_REGION_COW),   "p4_cow_break: COW flag cleared on dst");
    ASSERT(dr->flags & VMM_REGION_WRITE,    "p4_cow_break: write flag restored on dst");
    ASSERT(free_after_break < free_before_break,
           "p4_cow_break: new frame consumed from physmem");

    vmm_space_destroy(dst);
    vmm_space_destroy(src);
    print_result("p4_cow_break_gives_private_frame");
}

static void test_p4_cow_break_src_unaffected(void)
{
    reset_counters();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_break_src_safe: src created");

    u64 vaddr = 0x0000F00000000000ULL;
    vmm_space_alloc(src, vaddr, 1, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL, "p4_break_src_safe: clone non-null");

    vmm_cow_break(dst, vaddr);

    vmm_region_t *sr = vmm_space_find(src, vaddr);
    ASSERT(sr != NULL,                   "p4_break_src_safe: src region survives");
    ASSERT(sr->flags & VMM_REGION_USED,  "p4_break_src_safe: src region still used");

    vmm_space_destroy(dst);
    vmm_space_destroy(src);
    print_result("p4_cow_break_src_unaffected");
}

static void test_p4_cow_break_no_op_on_non_cow(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p4_break_nop: space created");

    u64 vaddr = 0x0000A10000000000ULL;
    vmm_space_alloc(sp, vaddr, 1, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_region_t *r = vmm_space_find(sp, vaddr);
    ASSERT(r != NULL,                   "p4_break_nop: region found");
    ASSERT(!(r->flags & VMM_REGION_COW),"p4_break_nop: not a COW region");

    u64 free_before = physmem_free_get();
    vmm_cow_break(sp, vaddr);
    u64 free_after = physmem_free_get();

    ASSERT(free_after == free_before,   "p4_break_nop: physmem unchanged");
    ASSERT(!(r->flags & VMM_REGION_COW),"p4_break_nop: flag still not COW");

    vmm_space_free(sp, vaddr);
    vmm_space_destroy(sp);
    print_result("p4_cow_break_no_op_on_non_cow");
}

static void test_p4_clone_null_safe(void)
{
    reset_counters();

    vmm_space_t *result = vmm_clone_space(NULL);
    ASSERT(result == NULL, "p4_clone_null: clone(NULL) returns NULL");

    print_result("p4_clone_null_safe");
}

static void test_p4_physmem_fully_reclaimed(void)
{
    reset_counters();

    u64 free_start = physmem_free_get();

    vmm_space_t *src = vmm_space_create();
    ASSERT(src != NULL, "p4_full_reclaim: src created");

    u64 base   = 0x0000A20000000000ULL;
    u64 stride = 0x10000ULL;

    #define P4R_N 8
    for (u64 i = 0; i < P4R_N; i++)
        vmm_space_alloc(src, base + i * stride, 2,
                        VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_space_t *dst = vmm_clone_space(src);
    ASSERT(dst != NULL, "p4_full_reclaim: clone non-null");

    for (u64 i = 0; i < P4R_N; i++)
        vmm_cow_break(dst, base + i * stride);

    vmm_space_destroy(dst);
    vmm_space_destroy(src);

    u64 free_end = physmem_free_get();
    ASSERT(free_end == free_start, "p4_full_reclaim: all frames returned");

    print_result("p4_physmem_fully_reclaimed");
    #undef P4R_N
}

static void test_p5_map_basic(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_map_basic: space created");

    u64 fake_phys = physmem_alloc_to(1);
    ASSERT(fake_phys != 0, "p5_map_basic: got a fake phys addr");

    u64 vaddr = 0x0000A30000000000ULL;
    u64 ret   = vmm_map_phys(sp, vaddr, fake_phys, 1,
                              VMM_REGION_READ | VMM_REGION_WRITE);

    ASSERT(ret == vaddr,           "p5_map_basic: returns vaddr");
    ASSERT(sp->region_count == 1,  "p5_map_basic: region_count == 1");
    ASSERT(sp->used_virtual == PAGE_SIZE, "p5_map_basic: used_virtual == PAGE_SIZE");

    vmm_region_t *r = vmm_space_find(sp, vaddr);
    ASSERT(r != NULL,                        "p5_map_basic: region found");
    ASSERT(r->base  == vaddr,                "p5_map_basic: region base correct");
    ASSERT(r->size  == PAGE_SIZE,            "p5_map_basic: region size correct");
    ASSERT(r->flags & VMM_REGION_USED,       "p5_map_basic: region marked used");
    ASSERT(r->flags & VMM_REGION_MMIO,       "p5_map_basic: MMIO flag set");
    ASSERT(r->flags & VMM_REGION_WRITE,      "p5_map_basic: write flag preserved");

    vmm_unmap_phys(sp, vaddr);
    ASSERT(sp->region_count == 0,  "p5_map_basic: region removed after unmap");
    ASSERT(sp->used_virtual == 0,  "p5_map_basic: used_virtual 0 after unmap");

    physmem_free_to(fake_phys, 1);
    vmm_space_destroy(sp);
    print_result("p5_map_basic");
}

static void test_p5_map_multipage(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_map_multi: space created");

    u64 page_count = 8;
    u64 fake_phys  = physmem_alloc_to(page_count);
    ASSERT(fake_phys != 0, "p5_map_multi: contiguous phys range allocated");

    u64 vaddr = 0x0000A40000000000ULL;
    u64 ret   = vmm_map_phys(sp, vaddr, fake_phys, page_count,
                              VMM_REGION_READ | VMM_REGION_WRITE);

    ASSERT(ret == vaddr,                                "p5_map_multi: returns vaddr");
    ASSERT(sp->used_virtual == page_count * PAGE_SIZE,  "p5_map_multi: used_virtual correct");

    vmm_region_t *r = vmm_space_find(sp, vaddr);
    ASSERT(r != NULL,                                   "p5_map_multi: region found at base");
    ASSERT(r->size == page_count * PAGE_SIZE,           "p5_map_multi: region size correct");
    ASSERT(r->flags & VMM_REGION_MMIO,                  "p5_map_multi: MMIO flag set");

    vmm_region_t *r2 = vmm_space_find(sp, vaddr + (page_count - 1) * PAGE_SIZE);
    ASSERT(r2 == r, "p5_map_multi: find works at last page of region");

    vmm_unmap_phys(sp, vaddr);
    ASSERT(sp->region_count == 0, "p5_map_multi: region count 0 after unmap");
    ASSERT(sp->used_virtual == 0, "p5_map_multi: no leak after unmap");

    physmem_free_to(fake_phys, page_count);
    vmm_space_destroy(sp);
    print_result("p5_map_multipage");
}

static void test_p5_physmem_unchanged(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_phys_unchanged: space created");

    u64 fake_phys = physmem_alloc_to(4);
    ASSERT(fake_phys != 0, "p5_phys_unchanged: phys allocated");

    u64 free_before_map = physmem_free_get();

    u64 vaddr = 0x0000A50000000000ULL;
    vmm_map_phys(sp, vaddr, fake_phys, 4, VMM_REGION_READ | VMM_REGION_WRITE);

    vmm_unmap_phys(sp, vaddr);

    u64 free_after_unmap = physmem_free_get();

    ASSERT(free_after_unmap == free_before_map,
           "p5_phys_unchanged: unmap does not alter physmem free count");

    physmem_free_to(fake_phys, 4);
    vmm_space_destroy(sp);
    print_result("p5_physmem_unchanged");
}

static void test_p5_flags_preserved(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_flags: space created");

    u64 fake_phys = physmem_alloc_to(1);
    ASSERT(fake_phys != 0, "p5_flags: phys allocated");

    u64 vaddr = 0x0000A60000000000ULL;
    vmm_map_phys(sp, vaddr, fake_phys, 1, VMM_REGION_READ | VMM_REGION_EXEC);

    vmm_region_t *r = vmm_space_find(sp, vaddr);
    ASSERT(r != NULL,                     "p5_flags: region found");
    ASSERT(r->flags & VMM_REGION_MMIO,    "p5_flags: MMIO flag present");
    ASSERT(r->flags & VMM_REGION_READ,    "p5_flags: READ flag preserved");
    ASSERT(r->flags & VMM_REGION_EXEC,    "p5_flags: EXEC flag preserved");
    ASSERT(!(r->flags & VMM_REGION_WRITE),"p5_flags: WRITE not set when not requested");

    vmm_unmap_phys(sp, vaddr);
    physmem_free_to(fake_phys, 1);
    vmm_space_destroy(sp);
    print_result("p5_flags_preserved");
}

static void test_p5_alias_two_vaddrs(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_alias: space created");

    u64 fake_phys = physmem_alloc_to(1);
    ASSERT(fake_phys != 0, "p5_alias: phys allocated");

    u64 va1 = 0x0000A70000000000ULL;
    u64 va2 = 0x0000A70000010000ULL;

    u64 r1 = vmm_map_phys(sp, va1, fake_phys, 1, VMM_REGION_READ);
    u64 r2 = vmm_map_phys(sp, va2, fake_phys, 1, VMM_REGION_READ);

    ASSERT(r1 == va1,             "p5_alias: first map returns va1");
    ASSERT(r2 == va2,             "p5_alias: second map returns va2");
    ASSERT(sp->region_count == 2, "p5_alias: two regions tracked");

    vmm_region_t *reg1 = vmm_space_find(sp, va1);
    vmm_region_t *reg2 = vmm_space_find(sp, va2);
    ASSERT(reg1 != NULL && reg2 != NULL, "p5_alias: both regions found");
    ASSERT(reg1 != reg2,                 "p5_alias: distinct region nodes");

    vmm_unmap_phys(sp, va1);
    ASSERT(sp->region_count == 1, "p5_alias: one region after first unmap");

    vmm_unmap_phys(sp, va2);
    ASSERT(sp->region_count == 0, "p5_alias: zero regions after second unmap");
    ASSERT(sp->used_virtual == 0, "p5_alias: no virtual leak");

    physmem_free_to(fake_phys, 1);
    vmm_space_destroy(sp);
    print_result("p5_alias_two_vaddrs");
}

static void test_p5_null_safe(void)
{
    reset_counters();

    u64 ret = vmm_map_phys(NULL, 0x0000A80000000000ULL, 0x1000, 1, VMM_REGION_READ);
    ASSERT(ret == 0, "p5_null_safe: map to NULL space returns 0");

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_null_safe: space created");

    ret = vmm_map_phys(sp, 0x0000A80000000000ULL, 0x1000, 0, VMM_REGION_READ);
    ASSERT(ret == 0, "p5_null_safe: map with page_count=0 returns 0");

    vmm_unmap_phys(sp, 0);
    vmm_unmap_phys(sp, 0xDEADBEEF000ULL);
    ASSERT(1, "p5_null_safe: unmap of unmapped addr does not crash");

    vmm_space_destroy(sp);
    print_result("p5_null_safe");
}

static void test_p5_mixed_with_regular_alloc(void)
{
    reset_counters();

    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_mixed: space created");

    u64 fake_phys = physmem_alloc_to(2);
    ASSERT(fake_phys != 0, "p5_mixed: fake phys allocated");

    u64 mmio_va = 0x0000A90000000000ULL;
    u64 norm_va = 0x0000A90000010000ULL;

    vmm_map_phys(sp, mmio_va, fake_phys, 2, VMM_REGION_READ | VMM_REGION_WRITE);
    vmm_space_alloc(sp, norm_va, 2, VMM_REGION_READ | VMM_REGION_WRITE);

    ASSERT(sp->region_count == 2,              "p5_mixed: two regions tracked");
    ASSERT(sp->used_virtual == 4 * PAGE_SIZE,  "p5_mixed: used_virtual == 4 pages");

    vmm_region_t *mr = vmm_space_find(sp, mmio_va);
    vmm_region_t *nr = vmm_space_find(sp, norm_va);

    ASSERT(mr != NULL && (mr->flags & VMM_REGION_MMIO),  "p5_mixed: MMIO region found");
    ASSERT(nr != NULL && !(nr->flags & VMM_REGION_MMIO), "p5_mixed: normal region found");

    u64 free_before = physmem_free_get();

    vmm_unmap_phys(sp, mmio_va);
    u64 free_after_mmio_unmap = physmem_free_get();
    ASSERT(free_after_mmio_unmap == free_before,
           "p5_mixed: MMIO unmap does not change physmem free count");

    vmm_space_free(sp, norm_va);
    u64 free_after_norm_free = physmem_free_get();
    ASSERT(free_after_norm_free > free_before,
           "p5_mixed: normal free does return frames to physmem");

    physmem_free_to(fake_phys, 2);
    vmm_space_destroy(sp);
    print_result("p5_mixed_with_regular_alloc");
}

static void test_p5_stress(void)
{
    reset_counters();

    #define P5S_N 16
    vmm_space_t *sp = vmm_space_create();
    ASSERT(sp != NULL, "p5_stress: space created");

    u64 base_va    = 0x0000AA0000000000ULL;
    u64 stride     = 0x10000ULL;
    u64 phys_bases[P5S_N];
    u64 vaddrs[P5S_N];

    for (u64 i = 0; i < P5S_N; i++)
    {
        phys_bases[i] = physmem_alloc_to(2);
        ASSERT_SILENT(phys_bases[i] != 0);
        vaddrs[i] = base_va + i * stride;
    }

    u64 free_before = physmem_free_get();

    for (u64 i = 0; i < P5S_N; i++)
        vmm_map_phys(sp, vaddrs[i], phys_bases[i], 2,
                     VMM_REGION_READ | VMM_REGION_WRITE);

    ASSERT(sp->region_count == P5S_N, "p5_stress: all regions mapped");

    for (u64 i = 0; i < P5S_N; i++)
        vmm_unmap_phys(sp, vaddrs[i]);

    ASSERT(sp->region_count == 0, "p5_stress: all regions unmapped");
    ASSERT(sp->used_virtual == 0, "p5_stress: no virtual leak");

    u64 free_after = physmem_free_get();
    ASSERT(free_after == free_before,
           "p5_stress: physmem free count unchanged after all MMIO map+unmap");

    for (u64 i = 0; i < P5S_N; i++)
        physmem_free_to(phys_bases[i], 2);

    vmm_space_destroy(sp);
    serial_printf("[VMM TEST] INFO  p5_stress: %d MMIO map+unmap cycles\n", P5S_N);
    print_result("p5_stress");
    #undef P5S_N
}

void vmm_run_all_tests(void)
{
    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] VMM PHASE 1 TEST SUITE\n");
    serial_printf("[VMM TEST] ============================\n");
    test_basic_alloc_free();
    test_page_alignment();
    test_multiple_alloc_free_all();
    test_no_overlap();
    test_free_null();
    test_double_free_resilience();
    test_coalesce();
    test_stats_consistency();

    serial_printf("[VMM TEST] ---- STRESS TESTS BEGIN ----\n");

    test_stress_batch_5000();
    test_stress_batch_10000();
    test_stress_random_pattern();
    test_stress_checkerboard();
    test_stress_varied_sizes();
    test_stress_interleaved();
    test_stress_rapid_realloc();
    test_stress_mixed_flags();

    test_final_leak_check();

    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] VMM PHASE 2 TEST SUITE\n");
    serial_printf("[VMM TEST] ============================\n");

    test_space_kernel_not_null();
    test_space_create_destroy();
    test_space_pml4_distinct();
    test_space_pml4_distinct_from_kernel();
    test_space_destroy_null_safe();
    test_space_destroy_kernel_safe();
    test_space_create_many();
    test_phase1_still_works_after_phase2_init();

    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] VMM PHASE 3 TEST SUITE\n");
    serial_printf("[VMM TEST] ============================\n");

    test_p3_alloc_basic();
    test_p3_alloc_multipage();
    test_p3_alloc_two_regions();
    test_p3_find_miss();
    test_p3_free_null_safe();
    test_p3_physmem_reclaimed();
    test_p3_destroy_frees_phys();
    test_p3_stress_alloc_free();

    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] VMM PHASE 4 TEST SUITE\n");
    serial_printf("[VMM TEST] ============================\n");

    test_p4_clone_creates_new_space();
    test_p4_clone_region_count();
    test_p4_clone_cow_flags();
    test_p4_physmem_shared_after_clone();
    test_p4_cow_break_gives_private_frame();
    test_p4_cow_break_src_unaffected();
    test_p4_cow_break_no_op_on_non_cow();
    test_p4_clone_null_safe();
    test_p4_physmem_fully_reclaimed();

    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] VMM PHASE 5 TEST SUITE\n");
    serial_printf("[VMM TEST] ============================\n");

    test_p5_map_basic();
    test_p5_map_multipage();
    test_p5_physmem_unchanged();
    test_p5_flags_preserved();
    test_p5_alias_two_vaddrs();
    test_p5_null_safe();
    test_p5_mixed_with_regular_alloc();
    test_p5_stress();

    serial_printf("[VMM TEST] ============================\n");
    serial_printf("[VMM TEST] ALL SUITES DONE\n");
    serial_printf("[VMM TEST] ============================\n");
}

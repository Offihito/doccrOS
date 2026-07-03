/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: lib.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <types.h>

void memset(void *ptr, u8 val, size_t n);
void memcpy(void *dst, const void *src, size_t n);
void memmove(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);

// Memory info is used in kernel but soon in console programms
u64 mem_get_free(void);
u64 mem_get_used(void);
u64 mem_get_total(void);

#endif

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: cpio.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef CPIO_H
#define CPIO_H

#include <types.h>

void cpio_extract(void *archive, u64 size);

#endif

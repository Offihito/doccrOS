/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: dirent.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include <stddef.h>

// this matches the linux dirent64 layout that getdents gives us
typedef struct
{
    unsigned long long  d_ino;
    long long           d_off;
    unsigned short      d_reclen;
    unsigned char       d_type;
    char                d_name[256];
} dirent_t;

// d_type values -- only the ones we actually use
#define DT_REG  8   // regular file
#define DT_DIR  4   // directory

long getdents(int fd, void *buf, size_t size);

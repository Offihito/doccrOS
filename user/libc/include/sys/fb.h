/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys/fb.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#pragma once

#include <stdint.h>

// mirror /kernel/devices/fb/fb0.h
#define FB_IOCTL_GET_WIDTH   0
#define FB_IOCTL_GET_HEIGHT  1
#define FB_IOCTL_GET_PITCH   2
#define FB_IOCTL_GET_BPP     3
#define FB_IOCTL_GET_SIZE    4
#define FB_IOCTL_GET_INFO    5
#define FB_IOCTL_MAP         6
#define FB_IOCTL_UNMAP       7
#define FB_IOCTL_FLUSH       8

typedef struct
{
    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    uint32_t    bpp;
    uint64_t    size;
} fb_info_t;
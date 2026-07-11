/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: fb0.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef FB0_H
#define FB0_H

#include <types.h>
#include <kernel/devices/device_init.h>

// sync to user/libc/include/sys/fb.h
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
    u32 width;
    u32 height;
    u32 pitch;
    u32 bpp;
    u64 size;
} fb_info_t;

extern device_handler fb0_device;

#endif
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: device_init.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

#include <types.h>

#define DEVICES_MAX_AMOUNT 256
#define VERSION_NUM(major, minor, patch, build) \
    ((major << 24) | (minor << 16) | (patch << 8) | build)

typedef struct device_handler {
    const char *name;
    const char *mount;      // mount point like /dev/console
    u32 version;        //actually not neccesary but cool

    // init/cleanup
    int (*init)(void);
    void (*fini)(void);

    // not used just for the file system in future
    //
    void *(*open)(const char *path);
    int (*read)(void *handle, void *buf, size_t count);
    int (*write)(void *handle, const void *buf, size_t count);

} device_handler;

void devices_init(void);
int device_register(device_handler *device);
void device_unregister(const char *name);
device_handler* device_find(const char *name);
int device_get_count(void);
device_handler* device_get_by_index(int idx);

#endif

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: vfs.h
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#ifndef VFS_H
#define VFS_H

#include <types.h>

#define VFS_NAME_MAX      64
#define VFS_MAX_CHILDREN  64   // TODO: do this dynamic
#define VFS_MAX_FILE_SIZE (64 * 1024 * 1024)  // 64 MiB
#define VFS_MAX_PATH      256


#define ROOT "/"

struct device_handler;

typedef enum {
    VFS_FILE,
    VFS_DIRECTORY,
    VFS_DEVICE
} vfs_type_t;

typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    vfs_type_t     type;

    u8      *data;      // only files get to have data, directories just hold hands xds
    u64     size;       // bytes used
    u64     capacity;
    u8      borrowed;

    struct device_handler *device; // only set when type == VFS_DEVICE

    struct vfs_node     *parent;
    struct vfs_node     *children[VFS_MAX_CHILDREN];
    int child_count;
} vfs_node_t;

// core
void vfs_init(void);
vfs_node_t *vfs_get_root(void);

vfs_node_t *vfs_create_node(vfs_node_t *parent, const char *name, vfs_type_t type);
vfs_node_t *vfs_mkdir(const char *path);
vfs_node_t *vfs_create_file(const char *path);
vfs_node_t *vfs_create_device(const char *path, struct device_handler *device);

int vfs_remove(const char *path);

vfs_node_t *vfs_find(const char *path);
vfs_node_t *vfs_find_child(vfs_node_t *dir, const char *name);

int vfs_write(vfs_node_t *node, const void *buf, u64 size);
int vfs_read(vfs_node_t *node, void *buf, u64 size);

void vfs_set_data(vfs_node_t *node, u8 *ptr, u64 size);

void  vfs_list(vfs_node_t *dir);


void rootfs_init(void);
void vfs_dump(void);

#endif

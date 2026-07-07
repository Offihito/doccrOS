/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: vfs.h
 * CREATED BY: --
 * MODIFIED BY: --
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


typedef enum {
    VFS_FILE,
    VFS_DIRECTORY
} vfs_type_t;

typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    vfs_type_t     type;

    u8      *data;      // only files get to have data, directories just hold hands
    u64     size;       // bytes actually used
    u64     capacity;   // bytes we bothered to allocate
    u8      borrowed;   // 1 = data points into external memory (e.g. CPIO archive), do not free

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

int vfs_remove(const char *path);

vfs_node_t *vfs_find(const char *path);
vfs_node_t *vfs_find_child(vfs_node_t *dir, const char *name);

int vfs_write(vfs_node_t *node, const void *buf, u64 size);
int vfs_read(vfs_node_t *node, void *buf, u64 size);

// zero-copy: point the node directly at existing memory (e.g. a Limine module).
// the caller is responsible for keeping that memory alive; it will NOT be freed.
void vfs_set_data(vfs_node_t *node, u8 *ptr, u64 size);

void  vfs_list(vfs_node_t *dir);


void rootfs_init(void);
void vfs_dump(void);

#endif

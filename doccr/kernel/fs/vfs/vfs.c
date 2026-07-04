/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: vfs.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "vfs.h"
#include <kernel/mem/meminclude.h>
#include <kernel/screen/lib/string.h>
#include <kernel/screen/lib/print.h>
#include <kernel/communication/serial.h>

static vfs_node_t *vfs_root = NULL;

static vfs_node_t *node_alloc(void)
{
    vfs_node_t *n = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    if (!n) return NULL;

    memset(n, 0, sizeof(vfs_node_t));
    return n;
}

void vfs_init(void)
{
    vfs_root = node_alloc();

    if (!vfs_root)
    {
        log("[VFS]", "could not allocate root node, rip\n");
        return;
    }

    str_copy(vfs_root->name, ROOT);
    vfs_root->type = VFS_DIRECTORY;
    vfs_root->parent = NULL;
    vfs_root->child_count = 0;

    log("[VFS]", "initialised vfs\n");
    //printf("root = %p\n", vfs_root);
}

vfs_node_t *vfs_get_root(void)
{
    return vfs_root;
}

vfs_node_t *vfs_create_node(vfs_node_t *parent, const char *name, vfs_type_t type)
{
    vfs_node_t *existing = vfs_find_child(parent, name);
    vfs_node_t *n = node_alloc();

    if (!parent || !name || name[0] == '\0') return NULL;
    if (parent->type != VFS_DIRECTORY) return NULL;
    if (existing) return existing;
    if (parent->child_count >= VFS_MAX_CHILDREN) return NULL;
    if (!n) return NULL;

    str_copy(n->name, name);

    n->type     = type;
    n->parent      = parent;
    n->child_count = 0;
    n->data     = NULL;
    n->size     = 0;
    n->capacity = 0;

    parent->children[parent->child_count++] = n;
/*
    printf(
        "p=%p cc=%d cn=%s\n",
        parent,
        parent->child_count,
        name
    );*/

    return n;
}

vfs_node_t *vfs_find_child(vfs_node_t *dir, const char *name)
{
    if (!dir || !name) return NULL;

    for (int i = 0; i < dir->child_count; i++)
    {
        if (str_equals(dir->children[i]->name, name))
        {
            return dir->children[i];
        }
    }
    return NULL;
}

vfs_node_t *vfs_find(const char *path)
{
    if (!vfs_root || !path) return NULL;
    if (path[0] == '\0' || str_equals(path, "/")) return vfs_root;

    vfs_node_t *cur = vfs_root;
    const char *p = path;
    char token[VFS_NAME_MAX];

    while (*p == '/') p++; // leading slashes are just noise

    while (*p)
    {
        int i = 0;

        while (*p && *p != '/' && i < VFS_NAME_MAX - 1) {
            token[i++] = *p++;
        }
        token[i] = '\0';
        if (i == 0) break;

        vfs_node_t *next = vfs_find_child(cur, token);
        if (!next) return NULL; // dead end, sorry

        cur = next;
        while (*p == '/') p++;
    }

    return cur;
}

vfs_node_t *vfs_mkdir(const char *path)
{
    if (!vfs_root || !path) return NULL;
    if (path[0] == '\0' || str_equals(path, "/")) return vfs_root;

    vfs_node_t *cur = vfs_root;

    const char *p = path;
    char token[VFS_NAME_MAX];

    while (*p == '/') p++;
    while (*p)
    {
        int i = 0;

        while (*p && *p != '/' && i < VFS_NAME_MAX - 1)
        {
            token[i++] = *p++;
        }
        token[i] = '\0';
        if (i == 0) break;

        vfs_node_t *next = vfs_find_child(cur, token);
        if (!next)
        {
            next = vfs_create_node(cur, token, VFS_DIRECTORY); // mkdir -p energy
            if (!next) return NULL;
        } else if (next->type != VFS_DIRECTORY)
        {
            return NULL; // somebody put a file where a folder should be, nope
        }

        cur = next;
        while (*p == '/') p++;
    }

    return cur;
}

vfs_node_t *vfs_create_file(const char *path)
{
    if (!vfs_root || !path) return NULL;

    int len = str_len(path);
    int last_slash = -1;

    for (int i = 0; i < len; i++)
    {
        if (path[i] == '/') last_slash = i;
    }

    char dirpath[VFS_MAX_PATH];
    char fname[VFS_NAME_MAX];

    if (last_slash < 0)
    {
        str_copy(dirpath, "/");
        str_copy(fname, path);
    } else if (last_slash == 0)
    {
        str_copy(dirpath, "/");
        str_copy(fname, path + 1);
    } else {
        int i = 0;
        for (; i < last_slash && i < VFS_MAX_PATH - 1; i++) dirpath[i] = path[i];
        dirpath[i] = '\0';
        str_copy(fname, path + last_slash + 1);
    }

    if (fname[0] == '\0') return NULL;

    vfs_node_t *dir = vfs_find(dirpath);
    if (!dir) dir = vfs_mkdir(dirpath); // THEN BUILD THAT DIR!!!
    if (!dir) return NULL;

    return vfs_create_node(dir, fname, VFS_FILE);
}

int vfs_remove(const char *path)
{
    vfs_node_t *node = vfs_find(path);
    vfs_node_t *parent = node->parent;

    if (!node || node == vfs_root) return -1; // no deleting the root if no permission
    if (!parent) return -1;

    if (
        node->type == VFS_DIRECTORY &&
        node->child_count > 0
    ){
        return -1;
    }

    int idx = -1;
    for (
        int i = 0;
        i < parent->child_count;
        i++) {
        if (
            parent->children[i] == node
        ) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return -1;


    for (
        int i = idx;
        i < parent->child_count - 1;
        i++
        ) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->child_count--;


    if (node->data) kfree((u64 *)node->data);
    kfree((u64 *)node);

    return 0;
}

int vfs_write(vfs_node_t *node, const void *buf, u64 size)
{
    if (!node || node->type != VFS_FILE || !buf) return -1;
    if (size > VFS_MAX_FILE_SIZE) size = VFS_MAX_FILE_SIZE; // no infinite files today xd

    if (!node->data || node->capacity < size)
    {
        u8 *newbuf = (u8 *)kmalloc(size);

        if (!newbuf) return -1;
        if (node->data) kfree((u64 *)node->data);

        node->data = newbuf;
        node->capacity = size;
    }

    memcpy(node->data, buf, size);
    node->size = size;

    return (int)size;
}

int vfs_read(vfs_node_t *node, void *buf, u64 size)
{
    if (!node || node->type != VFS_FILE || !buf) return -1;

    u64 to_copy = (size < node->size) ? size : node->size;
    if (to_copy > 0) memcpy(buf, node->data, to_copy);

    return (int)to_copy;
}

void vfs_list(vfs_node_t *dir)
{
    if (!dir) return;

    if (dir->type != VFS_DIRECTORY)
    {
        print(dir->name, white());
        print("\n", white());
        return;
    }

    for (int i = 0; i < dir->child_count; i++)
    {
        vfs_node_t *c = dir->children[i];

        if (c->type == VFS_DIRECTORY)
        {
            print(c->name, cyan());
            print("/\n", cyan());
        } else
        {
            print(c->name, white());
            print("\n", white());
        }
    }
}

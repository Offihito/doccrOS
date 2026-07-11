/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: lookup.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "vfs.h"
#include <kernel/communication/serial.h>

// two spaces per depth level, nothing fancy, just enough to see whos a child of who
static void dump_indent(int depth)
{
    for (int i = 0; i < depth; i++) printf("    ");
}

static void dump_node(vfs_node_t *node, int depth)
{
    if (!node) return;

    dump_indent(depth);

    if (node->type == VFS_DIRECTORY) {
        printf("%s/\n", node->name);

        for (int i = 0; i < node->child_count; i++) {
            dump_node(node->children[i], depth + 1);
        }
    } else if (node->type == VFS_DEVICE)
    {
        printf("%s (device)\n", node->name);
    } else
    {
        printf("%s (%llu bytes)\n", node->name, node->size);
    }
}

void vfs_dump(void)
{
    vfs_node_t *root = vfs_get_root();
    if (!root)
    {
        printf("[VFS] tree is empty, theres literally nothing to dump\n");
        return;
    }

    //printf("r=%p cc =%d\n",root, root->child_count);

    printf("[VFS] tree dump meow :3\n");
    printf("/\n");

    for (int i = 0; i < root->child_count; i++) {
        dump_node(root->children[i], 1);
    }

    printf("[VFS] seems like this is the end :)\n");
}

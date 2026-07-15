/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: mouse.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */


#include "mouse.h"
#include "../names.h"
#include <kernel/mem/lib.h>

#define MOUSE_QUEUE_SIZE 128

typedef struct {
    volatile input_event_t buf[MOUSE_QUEUE_SIZE];
    volatile u32 head;
    volatile u32 tail;
} mouse_queue_t;

static mouse_queue_t queue;

void mouse_push_event(const input_event_t *ev)
{
    if (!ev) return;
    u32 next = (queue.head + 1) % MOUSE_QUEUE_SIZE;
    if (next == queue.tail) return;
    queue.buf[queue.head] = *ev;
    queue.head = next;
}

static int mouse_read(void *handle, void *buf, size_t count)
{
    (void)handle;
    u8 *dst = buf;
    size_t written = 0;
    while (written + sizeof(input_event_t) <= count && queue.head != queue.tail) {
        input_event_t ev = (input_event_t)queue.buf[queue.tail];
        queue.tail = (queue.tail + 1) % MOUSE_QUEUE_SIZE;
        memcpy(dst + written, &ev, sizeof(ev));
        written += sizeof(ev);
    }
    return (int)written;
}

static int mouse_module_init(void)
{
    queue.head = queue.tail = 0;
    return 0;
}

static void mouse_module_fini(void)
{
    queue.head = queue.tail = 0;
}

device_handler mouse_module = {
    .name = MOUSE_NAME,
    .mount = MOUSE_MOUNT,
    .version = MOUSE_VERSION,
    .init = mouse_module_init,
    .fini = mouse_module_fini,
    .open = NULL,
    .close = NULL,
    .read = mouse_read,
    .write = NULL,
    .ioctl = NULL,
};

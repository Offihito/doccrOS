/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: kbd.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "kbd.h"
#include "../names.h"
#include <kernel/mem/lib.h>

#define KBD_QUEUE_SIZE 64

typedef struct {
    volatile input_event_t    buf[KBD_QUEUE_SIZE];
    volatile u32    head;
    volatile u32    tail;
} kbd_queue_t;

static kbd_queue_t    queue;

void kbd_push_event(const input_event_t *ev)
{
    if (!ev) return;

    u32 next = (queue.head + 1) % KBD_QUEUE_SIZE;
    if (next == queue.tail) return;

    queue.buf[queue.head] = *ev;
    queue.head = next;
}

int kbd_has_event(void)
{
    return queue.head != queue.tail;
}

int kbd_read_event(input_event_t *out)
{
    if (!kbd_has_event()) return 0;

    *out = (input_event_t)queue.buf[queue.tail];
    queue.tail = (queue.tail + 1) % KBD_QUEUE_SIZE;
    return 1;
}

u64 kbd_read_events(void *buf, u64 size)
{
    u8 *dst = (u8 *)buf;
    u64 written = 0;

    while (written  + sizeof(input_event_t) <= size)
    {
        input_event_t ev;
        if (!kbd_read_event(&ev)) break;

        memcpy(dst  + written, &ev, sizeof(input_event_t));
        written += sizeof(input_event_t);
    }

    return written;
}

static int kbd_read(void *handle, void *buf, size_t count)
{
    (void)handle;
    return (int)kbd_read_events(buf, count);
}

static int kbd_module_init(void)
{
    queue.head  = 0;
    queue.tail  = 0;
    return 0;
}

static void kbd_module_fini(void)
{
    queue.head  = 0;
    queue.tail  = 0;
}

device_handler kbd_module =
{
    .name    = KBD_NAME,
    .mount   = KBD_MOUNT,
    .version = KBD_VERSION,
    .init    = kbd_module_init,
    .fini    = kbd_module_fini,
    .open    = NULL,
    .close   = NULL,
    .read    = kbd_read,
    .write   = NULL,
    .ioctl   = NULL,
};

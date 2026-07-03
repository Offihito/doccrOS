

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: keyboard.h
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <types.h>
#include <kernel/devices/device_init.h>
#include <kernel/arch/x86_64/exceptions/irq.h>

#define KEY_BUFFER_SIZE 256

typedef struct {
    volatile u8  buf[KEY_BUFFER_SIZE];
    volatile u32 head;
    volatile u32 tail;
} key_ring_t;

void keyboard_init(void);
void keyboard_fini(void);

int  keyboard_has_key(void);
char keyboard_get_key(void);

extern device_handler keyboard_module;

#endif

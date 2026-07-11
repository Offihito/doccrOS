/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: kbd.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef INPUT_KBD_H
#define INPUT_KBD_H

#include "input.h"
#include <kernel/devices/device_init.h>

void kbd_push_event(const input_event_t *ev);
int kbd_has_event(void);
int kbd_read_event(input_event_t *out);
u64 kbd_read_events(void *buf, u64 size);

extern device_handler kbd_module;

#endif

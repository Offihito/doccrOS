/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: mouse.h
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#ifndef INPUT_MOUSE_H
#define INPUT_MOUSE_H

#include "input.h"
#include <kernel/devices/device_init.h>

void mouse_push_event(const input_event_t *ev);
extern device_handler mouse_module;

#endif

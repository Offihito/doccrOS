/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: input.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef INPUT_H
#define INPUT_H

#include <types.h>
#include "keycodes.h"


typedef enum {
    INPUT_EV_KEY  = 0,
    INPUT_EV_REL,
    INPUT_EV_ABS,
} input_event_type_t;

#define INPUT_MOD_SHIFT (1 << 0)
#define INPUT_MOD_CTRL (1 << 1)
#define INPUT_MOD_ALT (1 << 2)
#define INPUT_MOD_CAPS (1 << 3)

typedef struct {
    input_event_type_t  type;
    u16    code;
    i32    value;
    u8    modifiers;
} input_event_t;

#endif
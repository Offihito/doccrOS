/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: ctrl.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "ctrl.h"
#include "kbd.h"
#include <kernel/screen/lib/log.h>

void input_ctrl_init(void)
{
    log("[INPUT]", "init input controller\n");
}

void input_report_key(u16 code, int pressed, u8 modifiers)
{
    input_event_t ev;
    ev.type  = INPUT_EV_KEY;
    ev.code  = code;
    ev.value = pressed ? 1 : 0;
    ev.modifiers = modifiers;

    input_dispatch(&ev);
}

void input_dispatch(const input_event_t *ev)
{
    switch (ev->type)
    {
        case INPUT_EV_KEY:
            kbd_push_event(ev);
            break;

        // mouse in future here ig
        case INPUT_EV_REL:
        case INPUT_EV_ABS:
        default:
            break;
    }
}

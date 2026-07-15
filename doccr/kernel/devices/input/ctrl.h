/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: ctrl.h
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#ifndef INPUT_CTRL_H
#define INPUT_CTRL_H

#include "input.h"

void input_ctrl_init(void);
void input_report_key(u16 code, int pressed, u8 modifiers);
void input_report_rel(u16 axis, i32 value);
void input_dispatch(const input_event_t *ev);

#endif

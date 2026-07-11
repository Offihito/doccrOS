
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: init.c
 * CREATED BY: Offihito
 * MODIFIED BY: emex
 *
 */

#include "init.h"

#include <kernel/devices/device_init.h>

#include <kernel/arch/x86_64/drivers/ps2/keyboard/keyboard.h>
#include <kernel/devices/input/ctrl.h>
#include <kernel/devices/input/kbd.h>
#include <kernel/devices/fb/fb0.h>

void kernel_devices_init(void)
{
 	input_ctrl_init();
    keyboard_init();

    device_register(&kbd_module);
    device_register(&fb0_device);
}


/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: init.c
 * CREATED BY: Offihito
 * MODIFIED BY: Offihito
 *
 */

#include "init.h"

#include <kernel/devices/device_init.h>

#include <kernel/arch/x86_64/drivers/ps2/ps2.h>
#include <kernel/devices/input/ctrl.h>
#include <kernel/devices/input/kbd.h>
#include <kernel/devices/input/mouse.h>
#include <kernel/devices/fb/fb0.h>
#include <kernel/screen/lib/log.h>
#include <kernel/communication/serial.h>

void kernel_devices_init(void)
{
 	input_ctrl_init();
    device_register(&kbd_module);
    device_register(&mouse_module);
    device_register(&fb0_device);

    keyboard_init();
    int mouse_status = ps2_mouse_init();
    if (mouse_status == 0)
        log("[PS2]", "mouse enabled on IRQ12\n");
    else
        serial_printf("[PS2] mouse initialization failed at stage %d\n", mouse_status);
}

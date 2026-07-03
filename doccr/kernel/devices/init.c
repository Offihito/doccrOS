
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: init.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include "init.h"

#include <kernel/devices/device_init.h>
#include <kernel/arch/x86_64/drivers/ps2/keyboard/keyboard.h>

void kernel_devices_init(void)
{
    device_register(&keyboard_module);
}

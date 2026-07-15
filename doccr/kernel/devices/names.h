/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: mouse.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#pragma once

#include "device_init.h"

#define FB0_NAME "framebuffer0" // theres only one framebuffer normaly lol
#define FB0_MOUNT "/dev/fb0"
#define FB0_VERSION VERSION_NUM(1, 0, 0, 0)

#define KBD_NAME "keyboard_dev0" // we have different keyboards which means its the first one (0)
#define KBD_MOUNT "/dev/kbd_event0" // its a event the userspace gets
#define KBD_VERSION VERSION_NUM(1, 0, 0, 0)

#define MOUSE_NAME "mouse_dev0"
#define MOUSE_MOUNT "/dev/mouse"
#define MOUSE_VERSION VERSION_NUM(1, 0, 0, 0)
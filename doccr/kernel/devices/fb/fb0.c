/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: keyboard.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "../names.h"

void fb0_init(void)
{

}

static int fb0_dev_init()
{
	//
}

device_handler fb0_device =
{
    .name    = FB0_NAME,
    .mount   = FB0_MOUNT,
    .version = FB0_VERSION,
    .init    = fb0_dev_init,
    .fini    = NULL,
    .open    = NULL,
    .close   = NULL,
    .read    = NULL,
    .write   = NULL,
    .ioctl   = NULL,
};

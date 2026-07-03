/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: layout.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "boot.h"
#include <kernel/screen/graphics.h>

static char bs_buf_bs1[BS_BUF_SIZE];
static char bs_buf_bs2[BS_BUF_SIZE];
static char bs_buf_bs3[BS_BUF_SIZE];
static char bs_buf_bs4[BS_BUF_SIZE];

void bootscreen_layout_init(void)
{
    u32 fw      = get_fb_width();
    u32 fh      = get_fb_height();
    u32 half    = fw / 2;
    u32 mid     = fh / 2;

    bs.ScreensVisible(BS1, 1);
    bs.ScreensVisible(BS2, 1);
    bs.ScreensVisible(BS3, 1);
    bs.ScreensVisible(BS4, 1);

    // BS1 top left
    bs.Screens[BS1].cursor_x    = 0;
    bs.Screens[BS1].cursor_y    = 0;
    bs.Screens[BS1].buffer      = bs_buf_bs1;
    bs.Screens[BS1].x           = 0;
    bs.Screens[BS1].y           = 0;
    bs.Screens[BS1].width       = half;
    bs.Screens[BS1].height      = mid;

    // BS2 top right
    bs.Screens[BS2].cursor_x    = 0;
    bs.Screens[BS2].cursor_y    = 0;
    bs.Screens[BS2].buffer      = bs_buf_bs2;
    bs.Screens[BS2].x           = half;
    bs.Screens[BS2].y           = 0;
    bs.Screens[BS2].width       = half;
    bs.Screens[BS2].height      = mid;

    // BS3 bottom left
    bs.Screens[BS3].cursor_x    = 0;
    bs.Screens[BS3].cursor_y    = 0;
    bs.Screens[BS3].buffer      = bs_buf_bs3;
    bs.Screens[BS3].x           = 0;
    bs.Screens[BS3].y           = mid;
    bs.Screens[BS3].width       = half;
    bs.Screens[BS3].height      = mid;

    // BS4 bottom right
    bs.Screens[BS4].cursor_x    = 0;
    bs.Screens[BS4].cursor_y    = 0;
    bs.Screens[BS4].buffer      = bs_buf_bs4;
    bs.Screens[BS4].x           = half;
    bs.Screens[BS4].y           = mid;
    bs.Screens[BS4].width       = half;
    bs.Screens[BS4].height      = mid;

    bs.SwitchScreen(BS1);
}

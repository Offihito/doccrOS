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
static u32 bs_pix_bs1[BS_MAX_PIXELS];
static u32 bs_pix_bs2[BS_MAX_PIXELS];
static u32 bs_pix_bs3[BS_MAX_PIXELS];
static u32 bs_pix_bs4[BS_MAX_PIXELS];

static void bs_size_clamp(u32 *width, u32 *height)
{
    if (*width == 0) { *height = 0; return; }
    if ((u64)(*width) * (*height) <= BS_MAX_PIXELS) return;

    *height = BS_MAX_PIXELS / (*width);
}

static void bs_setup_screen
(
    int     screen,
    u32     *pixels,
    char    *buffer,

    u32  x,
    u32  y,
    u32  width,
    u32  height
) {
    bs_size_clamp(&width, &height);

    bs.Screens[screen].cursor_x    = 0;
    bs.Screens[screen].cursor_y    = 0;
    bs.Screens[screen].buffer      = buffer;
    bs.Screens[screen].x           = x;
    bs.Screens[screen].y           = y;
    bs.Screens[screen].width       = width;
    bs.Screens[screen].height      = height;
    bs.Screens[screen].pixels      = pixels;
    bs.Screens[screen].pixel_count = width * height;
}

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
    bs_setup_screen(
    	BS1, //screen
     	bs_pix_bs1, //pixels
      	bs_buf_bs1, // buffer
       	0,    //x
        0,    //y
        half, // w
        fh    // h
    );

    // BS2 top right
    bs_setup_screen(
    	BS2,
     	bs_pix_bs2,
      	bs_buf_bs2,
       	half,
        0,
        half,
        mid
    );

    // BS3 whole screen for fb0 device
    bs_setup_screen(
    	BS3,
     	bs_pix_bs3,
      	bs_buf_bs3,
       	0,
        0,
        fw,
        fh
    );

    // BS4 also for userspace ig but idk
    bs_setup_screen(
    	BS4,
     	bs_pix_bs4,
      	bs_buf_bs4,
       	half,
        mid,
        half,
        fh - mid
    );

    bs.SwitchScreen(BS1);
}

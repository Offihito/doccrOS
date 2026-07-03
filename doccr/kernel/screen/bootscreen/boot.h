/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: boot.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef BOOT_H
#define BOOT_H

#include <types.h>

// screen ids, nothing fancy
#define BS1     0
#define BS2     1
#define BS3     2
#define BS4     3

#define BS_COUNT 4

#define BS_BUF_SIZE 4096 // per screen

typedef struct
{
    char    *buffer;   // the layout
    u32     buf_len;

    u32     cursor_x;
    u32     cursor_y;

    u32     x, y;          // from topleft corber
    u32     width, height; // to right down corner

    u32     visible; // when 0 it doesnt bother drawing
} bs_screen_t;

// so we have bs. namespace
typedef struct
{
    bs_screen_t Screens[BS_COUNT];

    void  (*Init)(void);
    void  (*SwitchScreen)(int screen);
    void  (*ScreensVisible)(int screen, int on);
    void  (*Print)(const char *str, u32 color);
    void  (*Putchar)(char c, u32 color);
    void  (*Clear)(int screen);

} bootscreen_api_t;

extern bootscreen_api_t bs;

void bootscreen_setup(void);
void bootscreen_layout_init(void);

#endif

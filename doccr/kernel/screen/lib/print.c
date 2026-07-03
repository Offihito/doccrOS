/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: print.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "print.h"
#include <kernel/screen/bootscreen/boot.h>

// print.c is basically just a thin wrapper around bs.* now
// old direct-framebuffer drawing moved to boot.c

void putchar(char c, u32 color)
{
    bs.Putchar(c, color);
}

void string(const char *str, u32 color)
{
    bs.Print(str, color);
}

void printInt(int value, u32 color)
{
    char buffer[12];
    IntToString(value, buffer);
    string(buffer, color);
}

void print(const char *str, u32 color)
{
    string(str, color);
}

void reset_cursor(void)
{
    //cursor_x = 0;
    //cursor_y = 0;
    // TODO: reset cursor of the currently active bs screen, not the old globals
}

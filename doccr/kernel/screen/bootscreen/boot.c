/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: boot.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "boot.h"
#include <kernel/screen/graphics.h>
#include <kernel/screen/font_8x8.h>
#include <kernel/mem/lib.h>

bootscreen_api_t bs; // the one and only instance

static int active_screen = BS1; // the screen for bs.Print()/bs.Putchar() rn
// in layout.c to change

static void bs_setpixel(bs_screen_t *scr, u32 x, u32 y, u32 color)
{
    if (!scr->pixels) return;
    if (x >= scr->width || y >= scr->height) return;

    scr->pixels[y * scr->width + x] = color;
}


static void bs_draw_glyph(bs_screen_t *scr, char c, u32 color)
{
    const u8 *glyph = font_8x8[(u8)c];
    u32 scale = get_font_scale();

    for (int dy = 0; dy < 8; dy++)
    {
        u8 row = glyph[dy];

        for (int dx = 0; dx < 8; dx++)
        {
            if (!(row & (1 << (7 - dx)))) continue;

            for (u32 sy = 0; sy < scale; sy++)
            {
                for (u32 sx = 0; sx < scale; sx++)
                {
                    bs_setpixel
                    (
                        scr,
                        scr->cursor_x + dx * scale + sx,
                        scr->cursor_y + dy * scale + sy,
                        color
                    );
                }
            }
        }
    }
}

static void bs_flush_rect(bs_screen_t *scr, u32 x, u32 y, u32 w, u32 h)
{
    if (!scr->pixels) return;

    u32 end_x = x + w;
    u32 end_y = y + h;

    if (end_x > scr->width)  end_x = scr->width;
    if (end_y > scr->height) end_y = scr->height;

    for (u32 yy = y; yy < end_y; yy++)
    {
        for (u32 xx = x; xx < end_x; xx++)
        {
            putpixel(scr->x + xx, scr->y + yy, scr->pixels[yy * scr->width + xx]);
        }
    }
}


static void bs_flush(bs_screen_t *scr)
{
    bs_flush_rect(scr, 0, 0, scr->width, scr->height);
}

static void bs_clear_area(bs_screen_t *scr)
{
    if (!scr->pixels) return;

    for (u32 i = 0; i < scr->pixel_count; i++) scr->pixels[i] = black();

    bs_flush(scr);
}

static void bs_scroll(bs_screen_t *scr)
{
    u32 scale        = get_font_scale();
    u32 line_h       = 8 * scale + 2 * scale;

    if (!scr->pixels  || line_h >= scr->height)
    {
        // no backbuffer or the screen is too tiny to fit even one line, just wipe it
        bs_clear_area(scr);
        scr->cursor_x  = 0;
        scr->cursor_y  = 0;
        return;
    }

    u32 keep_rows    = scr->height - line_h;

    memmove
    (
        scr->pixels,
        scr->pixels  + line_h * scr->width,
        keep_rows * scr->width * sizeof(u32)
    );

    // black out the freshly exposed rows at the bottom
    u32 *bottom      = scr->pixels + keep_rows * scr->width;
    u32 bottom_len   = line_h * scr->width;

    for (u32 i = 0; i < bottom_len; i++) bottom[i] = black();

    bs_flush(scr);

    scr->cursor_x    = 0;
    scr->cursor_y    = keep_rows;
}

static void bs_putchar(char c, u32 color)
{
    bs_screen_t *scr = &bs.Screens[active_screen];
    if (!scr->visible) return;

    u32 scale      = get_font_scale();
    u32 char_w     = 8 * scale;
    u32 char_h     = 8 * scale;
    u32 line_h     = char_h + 2 * scale;

    if (c == '\n')
    {
        scr->cursor_x = 0;
        scr->cursor_y += line_h;
    }
    else
    {
        if (scr->cursor_x + char_w >= scr->width)
        {
            scr->cursor_x = 0;
            scr->cursor_y += line_h;
        }

        bs_draw_glyph(scr, c, color);
        bs_flush_rect(
        	scr,
         	scr->cursor_x,
          	scr->cursor_y,
           	char_w,
            char_h
        );
        scr->cursor_x += char_w;
    }

    if (
        scr->cursor_y + line_h >= scr->height
    ) bs_scroll(scr); // ran out of space :(

    // keeps a copy in log from boot
    if (scr->buffer && scr->buf_len < BS_BUF_SIZE - 1)
    {
        scr->buffer[scr->buf_len++]     = c;
        scr->buffer[scr->buf_len]       = '\0';
    }
}

static void bs_print(const char *str, u32 color)
{
    for (
        size_t i = 0;
        str[i];
        i++
    ) {
        bs_putchar(str[i], color);
    }
}

static void bs_putpixel(u32 x, u32 y, u32 color)
{
    bs_screen_t *scr   = &bs.Screens[active_screen];
    if (!scr->visible) return;

    bs_setpixel(scr, x, y, color);
    putpixel(scr->x + x, scr->y + y, color);
}

static void bs_switch_screen(int screen)
{
    // so it doesnt crash when a wrong input is given
    if (
        screen < 0 || screen >= BS_COUNT
    ) return;

    active_screen = screen;
}

static void bs_screens_visible(int screen, int on)
{
    if (
        screen < 0 || screen >= BS_COUNT
    )return;

    bs.Screens[screen].visible = on;
}

static void bs_clear(int screen)
{
    if (screen < 0 || screen >= BS_COUNT) return;

    bs_screen_t *scr = &bs.Screens[screen];
    bs_clear_area(scr);

    scr->cursor_x     = 0;
    scr->cursor_y     = 0;
    scr->buf_len      = 0;
    if (scr->buffer) scr->buffer[0] = '\0';
}

static void bs_init(void)
{
    memset(bs.Screens, 0, sizeof(bs.Screens));
    active_screen = BS1;
}

void bootscreen_setup(void)
{
    bs.Init               = bs_init;
    bs.SwitchScreen       = bs_switch_screen;
    bs.ScreensVisible     = bs_screens_visible;
    bs.Print              = bs_print;
    bs.Putchar            = bs_putchar;
    bs.Clear              = bs_clear;
    bs.Putpixel           = bs_putpixel;

    bs.Init();
}

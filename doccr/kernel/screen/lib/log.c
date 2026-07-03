/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: log.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "log.h"
#include "print.h"

static u32 get_tag_color(log_level_t level)
{
    switch (level) {
        case LSUCCESS:     return green();
        case LWARNING:     return yellow();
        case LERROR:       return red();
        case LWHITE:       return white();
        case LNORMAL:
        default:       return green(); // just the front
    }
}

static u32 get_msg_color(log_level_t level)
{
    switch (level) {
        case LSUCCESS:     return green();
        case LWARNING:     return yellow();
        case LERROR:       return red();
        case LWHITE:       return white();
        case LNORMAL:
        default:           return white(); // while message still is white
    }
}

static int last_tag_width = 0;

void log_message(const char *tag, const char *message, log_level_t level)
{
    if (!tag || !message) return;

    last_tag_width = str_len(tag) + 1;

    #if BOOTUP_VISUALS == 0
        u32 tag_color = get_tag_color(level);
        u32 msg_color = get_msg_color(level);

        print(tag, tag_color);
        print(" ", tag_color);
        print(message, msg_color);
    #endif
}

void log_continue(const char *message)
{
    if (!message)
        return;

    for (
        int i = 0;
        i < last_tag_width;
        i++
    ) {
        print(" ", white());
    }

    print(message, white());
}

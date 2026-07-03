/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: log.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef LOG_H
#define LOG_H

#include <types.h>
#include "../colors.h"

// if not set
#ifndef BOOTUP_VISUALS
#define BOOTUP_VISUALS 0
#endif

typedef enum {
    LNORMAL = 0,
    LSUCCESS,
    LWARNING,
    LERROR,
    LWHITE
} log_level_t;

// shortcuts
#define d       LNORMAL
#define success LSUCCESS
#define warning LWARNING
#define error   LERROR
#define wht     LWHITE
// default LNORMAL
#define log(tag, message, ...) log_message(tag, message, _LOG_LEVEL(__VA_ARGS__))
#define _LOG_LEVEL(...) _LOG_LEVEL_IMPL(_, ##__VA_ARGS__, LNORMAL, LNORMAL)
#define _LOG_LEVEL_IMPL(a, b, c, ...) c

void log_message(const char *tag, const char *message, log_level_t level);
void log_continue(const char *message);

#endif

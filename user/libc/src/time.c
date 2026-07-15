/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: time.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include <time.h>

time_t time(time_t *result) {
    struct timespec now;

    if (clock_gettime(CLOCK_REALTIME, &now) < 0) {
        return (time_t)-1;
    }
    if (result) {
        *result = now.tv_sec;
    }
    return now.tv_sec;
}

clock_t clock(void) {
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        return (clock_t)-1;
    }
    return (clock_t)(now.tv_sec * CLOCKS_PER_SEC + now.tv_nsec / 1000);
}

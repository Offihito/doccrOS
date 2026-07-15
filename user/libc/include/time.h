/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: time.h
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#pragma once

#include <stdint.h>

typedef long time_t;
typedef long clock_t;

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

#define CLOCKS_PER_SEC 1000000L
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

int clock_gettime(int clock_id, struct timespec *result);
time_t time(time_t *result);
clock_t clock(void);

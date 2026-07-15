/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: math.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include <math.h>

/* Compact approximation used only to construct Doom's view-angle table. */
static double atan_unit(double x)
{
    return x * (0.7853981633974483 + 0.273 * (1.0 - x));
}

double atan(double x)
{
    int negative = x < 0.0;
    if (negative) x = -x;
    double result = x > 1.0
        ? 1.5707963267948966 - atan_unit(1.0 / x)
        : atan_unit(x);
    return negative ? -result : result;
}

double fabs(double x)
{
    return x < 0.0 ? -x : x;
}

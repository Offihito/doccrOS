/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: assert.h
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#pragma once
#include <stdlib.h>
#define assert(x) ((x)?(void)0:exit(1))

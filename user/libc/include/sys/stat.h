/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: sys/stat.h
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#pragma once
#include <sys/types.h>
struct stat { mode_t st_mode; off_t st_size; };
int mkdir(const char *, mode_t);

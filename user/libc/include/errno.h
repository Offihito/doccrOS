/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: errno.h
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#pragma once
int *__errno_location(void);
#define errno (*__errno_location())
#define EIO 5
#define EISDIR 21
#define ENOENT 2
#define ENOMEM 12

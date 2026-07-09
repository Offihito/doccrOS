#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 emex-foundation
#
# FILE: Makefile
# CREATED BY: emex
# MODIFIED BY: Offihito
#
#

all:
	@$(MAKE) -C libc

	@$(MAKE) -C bin/hello

	@$(MAKE) -C bin/fork_test

	@$(MAKE) -C bin/syscall_test

clean:
	@$(MAKE) -C libc clean

	@$(MAKE) -C bin/hello clean

	@$(MAKE) -C bin/fork_test clean

	@$(MAKE) -C bin/syscall_test clean

.PHONY: all clean

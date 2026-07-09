#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 doccrLabs
#
# PROJECT: doccrOS
# FILE: Makefile
# CREATED BY: emex
# MODIFIED BY: --
#
#


CC  := x86_64-elf-gcc
AS  := nasm
AR  := x86_64-elf-ar

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -Iinclude

ASFLAGS := -f elf64

SRCS_C := src/string.c src/printf.c src/syscall.c
OBJS_C := $(SRCS_C:src/%.c=build/%.o)

all: build/crt0.o build/libc.a

build/crt0.o: src/crt0.asm | build
	$(AS) $(ASFLAGS) $< -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/libc.a: $(OBJS_C) | build
	$(AR) rcs $@ $(OBJS_C)

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean
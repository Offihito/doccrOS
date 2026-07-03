/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: serial.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "serial.h"
#include <kernel/arch/hal/ports.h>
#include <stdarg.h>

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

int serial_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
    while (!serial_ready());
    outb(COM1, c);
}

void serial_puts(const char *str) {
    if (!str) return;
    while (*str) {
        serial_putchar(*str);
        str++;
    }
}

static void print_u64(u64 num, int base)
{
    char buf[64];
    int i = 0;

    if (num == 0) {
        serial_putchar('0');
        return;
    }

    while (num > 0) {
        int digit = (int)(num % (u64)base);
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= (u64)base;
    }

    while (i > 0) {
        serial_putchar(buf[--i]);
    }
}

static void print_i64(i64 num) {
    if (num < 0) {
        serial_putchar('-');
        num = -num;
    }
    print_u64((u64)num, 10);
}

static void print_hexa32(u32 num) {
    serial_puts("0x");
    char buf[8];
    for (int i = 7; i >= 0; i--) {
        int digit = (num >> (i * 4)) & 0xF;
        buf[7 - i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    for (int i = 0; i < 8; i++) {
        serial_putchar(buf[i]);
    }
}

static void print_ptr(void *ptr) {
    serial_puts("0x");
    u64 val = (u64)ptr;
    char buf[16];
    for (int i = 15; i >= 0; i--) {
        int digit = (int)((val >> (i * 4)) & 0xF);
        buf[15 - i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    for (int i = 0; i < 16; i++) {
        serial_putchar(buf[i]);
    }
}

void serial_printf(const char *format, ...)
{
    if (!format) return;

    va_list args;
    va_start(args, format);

    while (*format)
    {
        if (*format != '%') {
            serial_putchar(*format);
            format++;
            continue;
        }

        format++;

        int is_long  = 0;
        int is_llong = 0;

        if (*format == 'l') {
            format++;
            is_long = 1;
            if (*format == 'l') {
                format++;
                is_llong = 1;
            }
        }

        switch (*format) {
            case 'd':
            case 'i':
                if (is_llong)
                    print_i64(va_arg(args, i64));
                else if (is_long)
                    print_i64((i64)va_arg(args, long));
                else
                    print_i64((i64)va_arg(args, int));
                break;

            case 'u':
                if (is_llong)
                    print_u64(va_arg(args, u64), 10);
                else if (is_long)
                    print_u64((u64)va_arg(args, unsigned long), 10);
                else
                    print_u64((u64)va_arg(args, unsigned int), 10);
                break;

            case 'x':
            case 'X':
                if (is_llong)
                    print_u64(va_arg(args, u64), 16);
                else
                    print_hexa32(va_arg(args, u32));
                break;

            case 'p':
                print_ptr(va_arg(args, void*));
                break;

            case 's':
                serial_puts(va_arg(args, const char*));
                break;

            case 'c':
                serial_putchar((char)va_arg(args, int));
                break;

            case '%':
                serial_putchar('%');
                break;

            default:
                serial_putchar('%');
                if (is_llong)  { serial_putchar('l'); serial_putchar('l'); }
                else if (is_long) { serial_putchar('l'); }
                serial_putchar(*format);
                break;
        }
        format++;
    }

    va_end(args);
}

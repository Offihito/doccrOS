/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS userspace
 * FILE: printf.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

static void out(const char *s, size_t len)
{
    write(1, s, len);
}

static void print_uint(unsigned long val, int base, int upper)
{
    char buf[32];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    if (val == 0)
    {
        buf[i++] = '0';
    } else
    {
        while (val > 0)
        {
            buf[i++] = digits[val % (unsigned)base];
            val /= (unsigned)base;
        }
    }

    while (i > 0)
    {
        char c = buf[--i];
        out(&c, 1);
    }
}

static void print_int(long val)
{
    if (val < 0)
    {
        char minus = '-';
        out(&minus, 1);
        val = -val;
    }
    print_uint((unsigned long)val, 10, 0);
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int written = 0;

    while (*fmt)
    {
        if (*fmt != '%')
        {
            out(fmt, 1);
            fmt++;
            written++;
            continue;
        }

        fmt++;

        switch (*fmt)
        {
            case 's': {
                const char *s = va_arg(ap, const char*);
                size_t len = strlen(s);
                out(s, len);
                written += (int)len;
                break;
            }
            case 'd':
            case 'i':
                print_int(va_arg(ap, int));
                break;
            case 'u':
                print_uint(va_arg(ap, unsigned int), 10, 0);
                break;
            case 'x':
                print_uint(va_arg(ap, unsigned int), 16, 0);
                break;
            case 'X':
                print_uint(va_arg(ap, unsigned int), 16, 1);
                break;
            case 'p': {
                void *p = va_arg(ap, void*);
                out("0x", 2);
                print_uint((unsigned long)p, 16, 0);
                break;
            }
            case 'c': {
                char c = (char)va_arg(ap, int);
                out(&c, 1);
                break;
            }
            case '%': {
                char c = '%';
                out(&c, 1);
                break;
            }
            default:
                out(fmt, 1);
                break;
        }

        fmt++;
    }

    va_end(ap);
    return written;
}

int puts(const char *s)
{
    size_t len = strlen(s);
    write(1, s, len);
    char nl = '\n';
    write(1, &nl, 1);
    return (int)(len + 1);
}
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: string.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include <string.h>

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

void *memset(void *dst, int val, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    for (size_t i = 0; i < n; i++) d[i] = (unsigned char)val;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *destination = dst;
    const unsigned char *source = src;

    if (destination < source) {
        for (size_t i = 0; i < n; i++) destination[i] = source[i];
    } else if (destination > source) {
        while (n > 0) {
            n--;
            destination[n] = source[n];
        }
    }

    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *left = a;
    const unsigned char *right = b;

    for (size_t i = 0; i < n; i++) {
        if (left[i] != right[i]) return left[i] - right[i];
    }
    return 0;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *bytes = s;
    for (size_t i = 0; i < n; i++) {
        if (bytes[i] == (unsigned char)c) return (void *)(bytes + i);
    }
    return NULL;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n > 0 && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    return n > 0 ? (unsigned char)*a - (unsigned char)*b : 0;
}

static unsigned char ascii_lower(unsigned char c)
{
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

int strncasecmp(const char *a, const char *b, size_t n)
{
    while (n > 0 && *a && ascii_lower(*a) == ascii_lower(*b)) {
        a++;
        b++;
        n--;
    }
    return n > 0 ? ascii_lower(*a) - ascii_lower(*b) : 0;
}

int strcasecmp(const char *a, const char *b)
{
    return strncasecmp(a, b, (size_t)-1);
}

char *strcpy(char *dst, const char *src)
{
    char *result = dst;
    while ((*dst++ = *src++)) { }
    return result;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    char *result = dst;
    while (n > 0 && *src) {
        *dst++ = *src++;
        n--;
    }
    while (n-- > 0) *dst++ = '\0';
    return result;
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t source_length = strlen(src);
    if (size > 0) {
        size_t copy_length = source_length < size - 1 ? source_length : size - 1;
        memcpy(dst, src, copy_length);
        dst[copy_length] = '\0';
    }
    return source_length;
}

size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t destination_length = strlen(dst);
    size_t source_length = strlen(src);
    if (destination_length < size)
        strlcpy(dst + destination_length, src, size - destination_length);
    return destination_length + source_length;
}

char *strcat(char *dst, const char *src)
{
    strcpy(dst + strlen(dst), src);
    return dst;
}

char *strchr(const char *s, int c)
{
    do {
        if (*s == (char)c) return (char *)s;
    } while (*s++);
    return NULL;
}

char *strrchr(const char *s, int c)
{
    const char *last_match = NULL;
    do {
        if (*s == (char)c) last_match = s;
    } while (*s++);
    return (char *)last_match;
}

char *strstr(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);
    if (needle_length == 0) return (char *)haystack;

    for (; *haystack; haystack++) {
        if (strncmp(haystack, needle, needle_length) == 0)
            return (char *)haystack;
    }
    return NULL;
}

char *strerror(int error)
{
    (void)error;
    return "I/O error";
}

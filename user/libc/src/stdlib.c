/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: stdlib.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define ARENA_SIZE (8UL * 1024 * 1024)
#define MALLOC_ALIGNMENT 16

typedef struct block {
    size_t size;
    struct block *next;
    int free;
} block_t;

static block_t *blocks;
static int libc_errno;

int *__errno_location(void) {
    return &libc_errno;
}

static size_t align_size(size_t size) {
    return (size + MALLOC_ALIGNMENT - 1) & ~(size_t)(MALLOC_ALIGNMENT - 1);
}

static int heap_init(void) {
    if (blocks) {
        return 1;
    }

    blocks = mmap(NULL, ARENA_SIZE, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANON, -1, 0);
    if (blocks == MAP_FAILED) {
        blocks = NULL;
        return 0;
    }

    blocks->size = ARENA_SIZE - sizeof(*blocks);
    blocks->next = NULL;
    blocks->free = 1;
    return 1;
}

static void split_block(block_t *block, size_t size) {
    block_t *remainder = (block_t *)((char *)(block + 1) + size);

    remainder->size = block->size - size - sizeof(*block);
    remainder->next = block->next;
    remainder->free = 1;

    block->next = remainder;
    block->size = size;
}

void *malloc(size_t size) {
    block_t *block;

    if (!size || !heap_init()) {
        return NULL;
    }

    size = align_size(size);
    for (block = blocks; block; block = block->next) {
        if (!block->free || block->size < size) {
            continue;
        }

        if (block->size >= size + sizeof(*block) + MALLOC_ALIGNMENT) {
            split_block(block, size);
        }

        block->free = 0;
        return block + 1;
    }

    return NULL;
}

void free(void *pointer) {
    block_t *block;

    if (!pointer) {
        return;
    }

    ((block_t *)pointer - 1)->free = 1;

    block = blocks;
    while (block && block->next) {
        if (block->free && block->next->free) {
            block->size += sizeof(*block) + block->next->size;
            block->next = block->next->next;
        } else {
            block = block->next;
        }
    }
}

void *calloc(size_t count, size_t size) {
    size_t total;
    void *pointer;

    if (size && count > (size_t)-1 / size) {
        return NULL;
    }

    total = count * size;
    pointer = malloc(total);
    if (pointer) {
        memset(pointer, 0, total);
    }
    return pointer;
}

void *realloc(void *pointer, size_t size) {
    block_t *old_block;
    void *new_pointer;

    if (!pointer) {
        return malloc(size);
    }
    if (!size) {
        free(pointer);
        return NULL;
    }

    old_block = (block_t *)pointer - 1;
    if (old_block->size >= size) {
        return pointer;
    }

    new_pointer = malloc(size);
    if (new_pointer) {
        memcpy(new_pointer, pointer, old_block->size);
        free(pointer);
    }
    return new_pointer;
}

int atoi(const char *text) {
    int sign = 1;
    int value = 0;

    while (*text == ' ') {
        text++;
    }
    if (*text == '-') {
        sign = -1;
        text++;
    }
    while (*text >= '0' && *text <= '9') {
        value = value * 10 + *text++ - '0';
    }
    return sign * value;
}

double atof(const char *text) {
    double value = 0;
    double place = 0.1;
    int sign = 1;

    while (*text == ' ') {
        text++;
    }
    if (*text == '-') {
        sign = -1;
        text++;
    }
    while (*text >= '0' && *text <= '9') {
        value = value * 10 + *text++ - '0';
    }
    if (*text == '.') {
        text++;
        while (*text >= '0' && *text <= '9') {
            value += (*text++ - '0') * place;
            place *= 0.1;
        }
    }
    return sign * value;
}

int abs(int value) {
    return value < 0 ? -value : value;
}

char *getenv(const char *name) {
    (void)name;
    return NULL;
}

int system(const char *command) {
    (void)command;
    return -1;
}

void exit(int status) {
    _exit(status);
}

char *strdup(const char *source) {
    size_t length = strlen(source) + 1;
    char *copy = malloc(length);

    if (copy) {
        memcpy(copy, source, length);
    }
    return copy;
}

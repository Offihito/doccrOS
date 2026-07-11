/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: keyboard.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "keyboard.h"
#include <kernel/arch/hal/ports.h>
#include <kernel/screen/bootscreen/boot.h>
#include <kernel/screen/colors.h>
#include <kernel/devices/names.h>

#define PS2_DATA    0x60
#define PS2_STATUS  0x64

#define SC_LSHIFT   0x2A
#define SC_RSHIFT   0x36
#define SC_CAPS     0x3A
#define SC_CTRL     0x1D
#define SC_ALT      0x38
#define SC_RELEASE  0x80
#define SC_EXTENDED 0xE0
#define SC_EXT_DEL  0x53

static const u8 sc_normal[128] = {
    [0x00]=0,   [0x01]=27,
    [0x02]='1', [0x03]='2', [0x04]='3', [0x05]='4', [0x06]='5',
    [0x07]='6', [0x08]='7', [0x09]='8', [0x0A]='9', [0x0B]='0',
    [0x0C]='-', [0x0D]='=', [0x0E]='\b',[0x0F]='\t',
    [0x10]='q', [0x11]='w', [0x12]='e', [0x13]='r', [0x14]='t',
    [0x15]='y', [0x16]='u', [0x17]='i', [0x18]='o', [0x19]='p',
    [0x1A]='[', [0x1B]=']', [0x1C]='\n',[0x1D]=0,
    [0x1E]='a', [0x1F]='s', [0x20]='d', [0x21]='f', [0x22]='g',
    [0x23]='h', [0x24]='j', [0x25]='k', [0x26]='l', [0x27]=';',
    [0x28]='\'', [0x29]='`',[0x2A]=0,   [0x2B]='\\',
    [0x2C]='z', [0x2D]='x', [0x2E]='c', [0x2F]='v', [0x30]='b',
    [0x31]='n', [0x32]='m', [0x33]=',', [0x34]='.', [0x35]='/',
    [0x36]=0,   [0x37]='*', [0x38]=0,   [0x39]=' ',
    [0x3A]=0,
};

static const u8 sc_shift[128] = {
    [0x00]=0,   [0x01]=27,
    [0x02]='!', [0x03]='@', [0x04]='#', [0x05]='$', [0x06]='%',
    [0x07]='^', [0x08]='&', [0x09]='*', [0x0A]='(', [0x0B]=')',
    [0x0C]='_', [0x0D]='+', [0x0E]='\b',[0x0F]='\t',
    [0x10]='Q', [0x11]='W', [0x12]='E', [0x13]='R', [0x14]='T',
    [0x15]='Y', [0x16]='U', [0x17]='I', [0x18]='O', [0x19]='P',
    [0x1A]='{', [0x1B]='}', [0x1C]='\n',[0x1D]=0,
    [0x1E]='A', [0x1F]='S', [0x20]='D', [0x21]='F', [0x22]='G',
    [0x23]='H', [0x24]='J', [0x25]='K', [0x26]='L', [0x27]=':',
    [0x28]='"', [0x29]='~', [0x2A]=0,   [0x2B]='|',
    [0x2C]='Z', [0x2D]='X', [0x2E]='C', [0x2F]='V', [0x30]='B',
    [0x31]='N', [0x32]='M', [0x33]='<', [0x34]='>', [0x35]='?',
    [0x36]=0,   [0x37]='*', [0x38]=0,   [0x39]=' ',
    [0x3A]=0,
};

static key_ring_t ring;
static volatile int shift_held   = 0;
static volatile int caps_lock    = 0;
static volatile int extended_key = 0;

static void ring_push(u8 c)
{
    u32 next = (ring.head + 1) % KEY_BUFFER_SIZE;
    if (next != ring.tail) {
        ring.buf[ring.head] = c;
        ring.head = next;
    }
}

static void keyboard_irq_handler(cpu_state_t *state)
{
    (void)state;

    if (!(inb(PS2_STATUS) & 1))
        return;

    u8 sc = inb(PS2_DATA);

    if (sc == SC_EXTENDED) {
        extended_key = 1;
        return;
    }

    if (sc & SC_RELEASE) {
        u8 make = sc & ~SC_RELEASE;
        if (!extended_key && (make == SC_LSHIFT || make == SC_RSHIFT))
            shift_held = 0;
        extended_key = 0;
        return;
    }

    if (extended_key) {
        extended_key = 0;
        if (sc == SC_EXT_DEL) {
            ring_push('\b');
            bs.Putchar('\b', COLOR_WHITE);
        }
        return;
    }

    if (sc == SC_LSHIFT || sc == SC_RSHIFT) { shift_held = 1; return; }
    if (sc == SC_CAPS)                       { caps_lock = !caps_lock; return; }
    if (sc == SC_CTRL || sc == SC_ALT)       { return; }

    if (sc >= 128) return;

    u8 c = shift_held ? sc_shift[sc] : sc_normal[sc];

    if (c >= 'a' && c <= 'z') {
        if (caps_lock ^ shift_held)
            c = (u8)(c - 'a' + 'A');
    }

    if (c) {
        ring_push(c);
        bs.Putchar((char)c, COLOR_WHITE);
    }
}

void keyboard_init(void)
{
    ring.head  = 0;
    ring.tail  = 0;
    shift_held   = 0;
    caps_lock    = 0;
    extended_key = 0;

    while (inb(PS2_STATUS) & 1)
        inb(PS2_DATA);

    irq_register_handler(1, keyboard_irq_handler);
}

void keyboard_fini(void)
{
    irq_unregister_handler(1);
}

int keyboard_has_key(void)
{
    return ring.head != ring.tail;
}

char keyboard_get_key(void)
{
    if (!keyboard_has_key())
        return 0;

    char c = (char)ring.buf[ring.tail];
    ring.tail = (ring.tail + 1) % KEY_BUFFER_SIZE;
    return c;
}

static int keyboard_module_init(void)
{
    keyboard_init();
    return 0;
}

static void keyboard_module_fini(void)
{
    keyboard_fini();
}

device_handler keyboard_module =
{
    .name    = KBD_NAME,
    .mount   = KBD_MOUNT,
    .version = KBD_VERSION,
    .init    = keyboard_module_init,
    .fini    = keyboard_module_fini,
    .open    = NULL,
    .close   = NULL,
    .read    = NULL,
    .write   = NULL,
    .ioctl   = NULL,
};

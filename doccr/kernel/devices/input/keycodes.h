/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: keycodes.h
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#ifndef INPUT_KEYCODES_H
#define INPUT_KEYCODES_H


//qwerty
typedef enum {
    KEY_NONE = 0,
    KEY_ESC,
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T,
    KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_LBRACKET, KEY_RBRACKET, KEY_ENTER,
    KEY_LCTRL,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,
    KEY_H, KEY_J, KEY_K, KEY_L,
    KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE,
    KEY_LSHIFT, KEY_BACKSLASH,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B,
    KEY_N, KEY_M,
    KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RSHIFT,
    KEY_KP_ASTERISK,
    KEY_LALT, KEY_SPACE, KEY_CAPSLOCK,
    KEY_DELETE,
} keycode_t;

#endif

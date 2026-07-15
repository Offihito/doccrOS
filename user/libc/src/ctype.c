/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 doccrLabs
 *
 * PROJECT: doccrOS
 * FILE: ctype.c
 * CREATED BY: Offihito
 * MODIFIED BY: --
 *
 */

#include <ctype.h>

int isalpha(int character) {
    return (character >= 'A' && character <= 'Z') ||
           (character >= 'a' && character <= 'z');
}

int isdigit(int character) {
    return character >= '0' && character <= '9';
}

int isalnum(int character) {
    return isalpha(character) || isdigit(character);
}

int isspace(int character) {
    return character == ' ' || character == '\t' || character == '\n' ||
           character == '\r' || character == '\f' || character == '\v';
}

int isupper(int character) {
    return character >= 'A' && character <= 'Z';
}

int islower(int character) {
    return character >= 'a' && character <= 'z';
}

int isprint(int character) {
    return character >= 32 && character < 127;
}

int isxdigit(int character) {
    return isdigit(character) ||
           (character >= 'a' && character <= 'f') ||
           (character >= 'A' && character <= 'F');
}

int toupper(int character) {
    return islower(character) ? character - 'a' + 'A' : character;
}

int tolower(int character) {
    return isupper(character) ? character - 'A' + 'a' : character;
}

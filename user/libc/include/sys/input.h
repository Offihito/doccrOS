#ifndef _SYS_INPUT_H
#define _SYS_INPUT_H

#include <stdint.h>

typedef enum {
    INPUT_EV_KEY = 0,
    INPUT_EV_REL,
    INPUT_EV_ABS,
} input_event_type_t;

#define INPUT_REL_X 0
#define INPUT_REL_Y 1
#define INPUT_BTN_LEFT 0x110
#define INPUT_BTN_RIGHT 0x111
#define INPUT_BTN_MIDDLE 0x112

typedef struct {
    input_event_type_t type;
    uint16_t code;
    int32_t value;
    uint8_t modifiers;
} input_event_t;

#endif

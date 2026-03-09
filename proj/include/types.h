#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

typedef uint8_t pin_t;

enum Event : uint32_t
{
    NONE_EVENT = 0,
    BUTTON_EVENT = 1 << 0,
};

enum class LedState : uint8_t {
    OFF,
    ON, 
    BLINK,
};

#endif
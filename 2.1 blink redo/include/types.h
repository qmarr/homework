#include <Arduino.h>

#ifndef TYPES_H
#define TYPES_H

typedef uint8_t pin_t;

constexpr pin_t LED_PIN{5};

enum class BLINK_SPEED
{
    FAST = 200,
    MEDIUM = 400,
    SLOW = 800,
    SLUGGISH = 1200
};

#endif

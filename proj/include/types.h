#include <Arduino.h>

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
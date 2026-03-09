#include <Arduino.h>

enum Event : uint32_t
{
    TIMER_EVENT = 1 << 0,
    BUTTON_EVENT = 1 << 1,
};
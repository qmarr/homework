#include <Arduino.h>

typedef uint8_t pin_t;

constexpr pin_t LED_PIN{5};

enum class LED_STATE
{
    TURN_OFF,
    TURN_ON,
    LED_STATES_NUM
};

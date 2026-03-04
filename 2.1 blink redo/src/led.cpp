#include "led.h"

void Led::init()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

Led::Led() : pin(LED_PIN)
{
    init();
}

Led::~Led()
{
}

void Led::setLedState(bool st)
{
    state = st;
}

bool Led::makeStateOpposite()
{
    state = !state;
    return state;
}

pin_t Led::getLedPin() const
{
    return pin;
}   

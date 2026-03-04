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

void Led::setLedState(LED_STATE st)
{
    state = st;
}

pin_t Led::getLedPin() const
{
    return pin;
}   

#include "led.h"

void Led::on()
{
    state = true;
    digitalWrite(pin, HIGH);
}

void Led::off()
{
    state = false;
    digitalWrite(pin, LOW);
}

void Led::init()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

Led::Led() : pin()
{

}

Led::Led(pin_t pin) : pin(pin)
{

}

Led::~Led()
{
}

void Led::setLedState(bool st)
{
    state = st;
    digitalWrite(pin, state ? HIGH : LOW);
}

bool Led::getLedState() const
{
    return state;
}

void Led::toggle()
{
    setLedState(!state);
}

pin_t Led::getLedPin() const
{
    return pin;
}   

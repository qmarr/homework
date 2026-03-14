#ifndef RELAY_H
#define RELAY_H

#include "Arduino.h"

class Relay
{
private:
    uint8_t pin;
    bool status;

public:
    void init();
    Relay();
    Relay(uint8_t p);
    ~Relay();
    void setStatus(bool st);
    bool getStatus() const;
};

inline void Relay::init()
{
    pinMode(pin, OUTPUT);
}

Relay::Relay()
{
}

inline Relay::Relay(uint8_t p) : pin(p)
{
}

Relay::~Relay()
{
}

inline void Relay::setStatus(bool st)
{
    status = st;
    digitalWrite(pin, (status == true) ? HIGH : LOW);
}

inline bool Relay::getStatus() const
{
    return status;
}

#endif
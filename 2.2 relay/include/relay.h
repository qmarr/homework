#ifndef RELAY_H
#define RELAY_H

#include "Arduino.h"

class Relay
{
private:
    uint8_t pin;
    bool status{false};
    uint64_t RLdelay{1000};
    uint64_t lastTick{0};

public:
    void init();
    Relay();
    Relay(uint8_t p);
    ~Relay();
    void setStatus(bool st);
    bool getStatus() const;
    void switchRL(); 
};

inline void Relay::init()
{
    pinMode(pin, OUTPUT);
}

Relay::Relay()
{
}

inline Relay::Relay(uint8_t p) : pin(p), status(false)
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

inline void Relay::switchRL()
{
    uint64_t curMillis = millis();

    if(curMillis - lastTick > RLdelay) {

        lastTick = curMillis;
        setStatus(!status);
    }
}

#endif
#include "types.h"

class Led
{
private:
    bool state{false};
    pin_t pin;
    uint64_t lastTick{0};
    uint16_t blinkPeriod{200};
public:
    void on();
    void off();
    void init();
    explicit Led();
    explicit Led(pin_t pin);
    explicit Led(pin_t pin, uint16_t blperiod);
    ~Led();
    void setLedState(bool st);
    bool getLedState() const;
    void toggle();
    pin_t getLedPin() const;
    void setLastTick(uint64_t lstTick);
    uint64_t getLastTick() const;
    uint16_t getBLinkPeriod() const;
};

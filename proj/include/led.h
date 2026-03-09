#include "types.h"

class Led
{
private:
    bool state{false};
    pin_t pin;

public:
    void on();
    void off();
    void init();
    explicit Led();
    explicit Led(pin_t pin);
    ~Led();
    void setLedState(bool st);
    bool getLedState() const;
    void toggle();
    pin_t getLedPin() const;
};

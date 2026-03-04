#include "types.h"

class Led
{
private:
    LED_STATE state{LED_STATE::TURN_OFF};
    pin_t pin{LED_PIN};

public:
    void init();
    Led();
    ~Led();
    void setLedState(LED_STATE st);
    pin_t getLedPin() const;
};

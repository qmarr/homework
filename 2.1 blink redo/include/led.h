#include "types.h"

class Led
{
private:
    bool state{false};
    pin_t pin{LED_PIN};

public:
    void init();
    Led();
    ~Led();
    void setLedState(bool st);
    bool makeStateOpposite();
    pin_t getLedPin() const;
};

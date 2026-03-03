#include <Arduino.h>

volatile uint32_t event_counter{0};
constexpr uint32_t MAX_CAP{100};
constexpr uint8_t ISR_LED{5};
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
bool led_state = false;

void doSomething()
{
    led_state = !led_state;
    digitalWrite(ISR_LED, led_state);
}

void IRAM_ATTR timer_isr(void *arg)
{
    event_counter++;
}

void setup()
{
    pinMode(ISR_LED, OUTPUT);
    digitalWrite(ISR_LED, LOW);
}

void loop()
{
    // тимчасово симулюємо “прихід подій”
    event_counter += 3;
    delay(10);

    uint32_t snapshot;

    // TODO: atomic read + reset
    portENTER_CRITICAL(&mux); // temp RTOS?
    snapshot = event_counter;
    event_counter = 0;
    portEXIT_CRITICAL(&mux);

    uint32_t n = (snapshot > MAX_CAP) ? MAX_CAP : snapshot;

    for (uint32_t i = 0; i < n; i++)
    {
        doSomething();
    }
}

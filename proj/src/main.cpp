#include <Arduino.h>

constexpr uint32_t MAX_CAP{100};
constexpr uint8_t ISR_LED{5};
constexpr uint32_t ALARM_TICKS{1000};

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
bool led_state = false;

volatile uint32_t event_counter{0};
hw_timer_t *timer{nullptr};

uint32_t handled{0};
uint32_t dropped_events{0};

void handleISR()
{
    led_state = !led_state;
    digitalWrite(ISR_LED, led_state);
}

void IRAM_ATTR timer_isr()
{
    event_counter++;
}

void setup()
{
    Serial.begin(115200);
    pinMode(ISR_LED, OUTPUT);
    digitalWrite(ISR_LED, LOW);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &timer_isr, true);
    timerAlarmWrite(timer, ALARM_TICKS, true);
    timerAlarmEnable(timer);
}

void loop()
{
    uint32_t snapshot;

    portENTER_CRITICAL(&mux); // RTOS, critical section
    snapshot = event_counter;
    event_counter = 0;
    portEXIT_CRITICAL(&mux);

    uint32_t n = (snapshot > MAX_CAP) ? MAX_CAP : snapshot;
    handled += n;
    if (snapshot > MAX_CAP)
        dropped_events += snapshot - MAX_CAP;
    
    for (uint32_t i = 0; i < n; i++)
    {
        if (handled > 500)
        {
            handleISR();
            Serial.print("Dropped events: ");
            Serial.println(dropped_events);
            handled -= 500;
        }
    }
}

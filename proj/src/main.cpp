#include "types.h"

constexpr uint32_t MAX_CAP{100};
constexpr uint8_t ISR_LED_PIN{5};
constexpr uint8_t BTN_PIN{15};
constexpr uint32_t ALARM_TICKS{1000}; // 1kHz

bool led_state = false;

volatile uint32_t events{0};
volatile uint32_t timer_ticks{0};
volatile uint32_t button_presses{0};

hw_timer_t *timer{nullptr};

uint32_t handled{0};
uint32_t dropped_events{0};

void handleTimerEvent(uint32_t ticks, uint32_t presses)
{
    Serial.print("ticks passed: ");
    Serial.println(ticks);
    Serial.print("presses happend: ");
    Serial.println(presses);
}

void handleBtnEvent()
{
    led_state = !led_state;
    digitalWrite(ISR_LED_PIN, led_state);
}

void IRAM_ATTR timer_isr()
{
    noInterrupts();
    events |= TIMER_EVENT;
    timer_ticks++;
    interrupts();
}

void IRAM_ATTR button_isr()
{
    noInterrupts();
    events |= BUTTON_EVENT;
    button_presses++;
    interrupts();
}

void setup()
{
    Serial.begin(115200);

    pinMode(ISR_LED_PIN, OUTPUT);
    digitalWrite(ISR_LED_PIN, LOW);

    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_PIN), &button_isr, FALLING);

    timer = timerBegin(0, 80, true);

    timerAttachInterrupt(timer, &timer_isr, true);
    timerAlarmWrite(timer, ALARM_TICKS, true);
    timerAlarmEnable(timer);
}

void loop()
{
    uint32_t pending{0}, btnSnap{0}, timerSnap{0};

    noInterrupts(); // critical section
    pending = events;
    btnSnap = button_presses;
    timerSnap = timer_ticks;
    events = 0;
    timer_ticks = 0;
    button_presses = 0;
    interrupts();

    if (pending & TIMER_EVENT)
        handleTimerEvent(timerSnap, btnSnap);

    if (pending & BUTTON_EVENT)
        handleBtnEvent();
 
}

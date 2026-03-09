#include "types.h"

constexpr uint32_t MAX_CAP{100};
constexpr uint8_t LED_PIN{5};
constexpr uint8_t BTN_PIN{15};
constexpr uint32_t ALARM_TICKS{1000}; // 1kHz

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

bool led_state = false;

volatile uint32_t events{0};
volatile uint32_t timer_ticks{0};
volatile uint32_t button_presses{0};

hw_timer_t *timer{nullptr};

LedState state{LedState::OFF};

void handleTimerEvent(uint32_t ticks)
{
    Serial.print("ticks passed: ");
    Serial.println(ticks);
}

void handleBtnEvent(uint32_t presses)
{
    Serial.print("presses happend: ");
    Serial.println(presses);
}

void toggleLED()
{
    uint32_t curTicks{millis()};
    static uint32_t lastTicks{0};

    if (curTicks - lastTicks >= 500)
    {
        lastTicks = curTicks;

        led_state = !led_state;
        digitalWrite(LED_PIN, led_state);
    }
}

void IRAM_ATTR timer_isr()
{
    portENTER_CRITICAL_ISR(&mux);
    timer_ticks++;
    portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR button_isr()
{
    portENTER_CRITICAL_ISR(&mux);
    button_presses++;
    portEXIT_CRITICAL_ISR(&mux);
}

void updateState(LedState &state, uint32_t events)
{
    switch (state)
    {
    case LedState::OFF:
        if (events == BUTTON_EVENT)
            state = LedState::ON;
        break;
    case LedState::ON:
        if (events == BUTTON_EVENT)
            state = LedState::BLINK;
        break;
    case LedState::BLINK:
        if (events == BUTTON_EVENT)
            state = LedState::OFF;
        break;
    }   
}

void applyState(LedState state)
{
    switch (state)
    {
    case LedState::OFF:
        digitalWrite(LED_PIN, LOW);
        break;
    case LedState::ON:
        digitalWrite(LED_PIN, HIGH);
        break;
    case LedState::BLINK:
        toggleLED();
        break;
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_PIN), &button_isr, FALLING);

    timer = timerBegin(0, 80, true);

    timerAttachInterrupt(timer, &timer_isr, true);
    timerAlarmWrite(timer, ALARM_TICKS, true);
    timerAlarmEnable(timer);
}

void loop()
{

    // todo events of isrs are mixed with ticks and presses
    // it's better to count presses or ticks without events for future.
    uint32_t pending{0}, tickSnap{0}, btnSnap{0};

    portENTER_CRITICAL_ISR(&mux); // critical section
    tickSnap = timer_ticks;
    btnSnap = button_presses;
    timer_ticks = 0;
    button_presses = 0;
    portEXIT_CRITICAL_ISR(&mux);

    if (btnSnap > 0)
        updateState(state, BUTTON_EVENT);
    applyState(state);
}

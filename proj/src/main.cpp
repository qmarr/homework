#include "types.h"
#include "led.h"

constexpr uint32_t BLINK_PERIOD_MS{500};
constexpr uint8_t LED_PIN{5};
constexpr uint8_t BTN_PIN{15};
constexpr uint32_t ALARM_TICKS{1000}; // 1kHz
constexpr uint32_t DEBOUNCE_MS{50};

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t timer_ticks{0};
volatile uint32_t button_presses{0};
volatile uint32_t last_button_tick{0};

hw_timer_t *timer{nullptr};

Led led{LED_PIN};
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

void IRAM_ATTR timer_isr()
{
    portENTER_CRITICAL_ISR(&mux);
    timer_ticks++;
    portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR button_isr()
{
    portENTER_CRITICAL_ISR(&mux);
    uint32_t now = timer_ticks; 

    if (now - last_button_tick >= DEBOUNCE_MS)
    {
        last_button_tick = now;
        button_presses++;
    }
    portEXIT_CRITICAL_ISR(&mux);
}

void updateState(LedState &state, Event event)
{
    switch (state)
    {
    case LedState::OFF:
        if (event == BUTTON_EVENT)
            state = LedState::ON;
        break;
    case LedState::ON:
        if (event == BUTTON_EVENT)
            state = LedState::BLINK;
        break;
    case LedState::BLINK:
        if (event == BUTTON_EVENT)
            state = LedState::OFF;
        break;
    }
}

void applyState(LedState state, uint32_t tickSnap, Led &led)
{

    static uint32_t blink_accum{0};
    switch (state)
    {
    case LedState::OFF:
        blink_accum = 0;
        led.off();
        break;
    case LedState::ON:
        blink_accum = 0;
        led.on();
        break;
    case LedState::BLINK:
        blink_accum += tickSnap;
        if(blink_accum >= BLINK_PERIOD_MS)
        {
            blink_accum -= BLINK_PERIOD_MS;
            led.toggle();
        }
        break;
    }
}

void setup()
{
    Serial.begin(115200);

    led.init();

    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_PIN), &button_isr, CHANGE);

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

    portENTER_CRITICAL(&mux); // critical section
    tickSnap = timer_ticks;
    btnSnap = button_presses;
    timer_ticks = 0;
    button_presses = 0;
    portEXIT_CRITICAL(&mux);

    if (btnSnap > 0)
        updateState(state, BUTTON_EVENT);
    applyState(state, tickSnap, led);
}

#include <Arduino.h>

typedef uint8_t pin_t;
void policeBlink(pin_t, pin_t, uint16_t);
constexpr pin_t LEDS_COUNT = 2;
constexpr pin_t LED_PINS[LEDS_COUNT]{17, 18};
constexpr pin_t BTN_PIN = 5;
constexpr pin_t BOOT_BTN = 0;

struct Button
{
  bool pressed;
  bool state;
  bool lastState;
  const pin_t btnPin;
  uint16_t lastDebounceTime = 0;
  uint16_t debounceDelay = 50;

  Button(bool pressed, bool state, bool lastState, pin_t pin)
      : pressed(pressed), state(state), lastState(lastState), btnPin(pin)
  {
  }
};

Button outBtn(false, false, HIGH, BTN_PIN);
Button bootBtn(false, false, HIGH, BOOT_BTN);

void setup()
{

  for (size_t i = 0; i < LEDS_COUNT; i++)
  {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  Serial.begin(115200);
}

void btnHandler(Button &btn)
{
  uint16_t reading = digitalRead(btn.btnPin);

  if (reading != btn.lastState)
  {
    btn.lastDebounceTime = millis();
  }

  if ((millis() - btn.lastDebounceTime) > btn.debounceDelay)
  {
    if (reading != btn.state)
    {
      btn.state = reading;
      if (btn.state == LOW)
      {
        btn.pressed = !btn.pressed;
        Serial.println("pressed");
      }
    }
  }
  btn.lastState = reading;
}

void loop()
{

  btnHandler(outBtn);
  btnHandler(bootBtn);

  if (outBtn.pressed)
  {

    policeBlink(LED_PINS[0], LED_PINS[1], 200);
  }
  else if (bootBtn.pressed)
  {

    policeBlink(LED_PINS[0], LED_PINS[1], 500);
  }

}

void policeBlink(pin_t firstPin, pin_t secondPin, uint16_t milisecondsDelay)
{

  uint32_t currMillis = millis();
  static uint8_t state = 0;

  static uint32_t lastTick = 0;

  if (currMillis - lastTick >= milisecondsDelay)
  {

    lastTick = currMillis;

    switch (state)
    {
    case 0:
      digitalWrite(firstPin, HIGH);
      digitalWrite(secondPin, LOW);
      break;
    case 1:
      digitalWrite(firstPin, LOW);
      digitalWrite(secondPin, HIGH);
      break;
    case 2:
      digitalWrite(firstPin, LOW);
      digitalWrite(secondPin, LOW);
      break;
    }

    state++;

    if (state > 2)
      state = 0;
  }
}
//when button pressed to stop blinker, It can stop while one of leds Lighted. Also program can be stopped by just touching button not pressing
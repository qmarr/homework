#include <Arduino.h>

typedef uint8_t pin_t;
void policeBlink(pin_t, pin_t);
constexpr pin_t LEDS_COUNT = 2;
constexpr pin_t LED_PINS[LEDS_COUNT]{17, 18};

void setup() {
  
  for (size_t i = 0; i < LEDS_COUNT; i++)
  {
    pinMode(LED_PINS[i], OUTPUT);
  }
}

void loop() {
  policeBlink(LED_PINS[0], LED_PINS[1]);
}


void policeBlink(pin_t firstPin, pin_t secondPin) {

  digitalWrite(firstPin, HIGH);
  delay(500);

  digitalWrite(firstPin, LOW);
  digitalWrite(secondPin, HIGH);
  delay(500);

  digitalWrite(secondPin, LOW);
}
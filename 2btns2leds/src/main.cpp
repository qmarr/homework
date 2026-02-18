#include <Arduino.h>

typedef uint8_t pin_t;
void policeBlink(pin_t, pin_t, uint16_t);
constexpr pin_t LEDS_COUNT = 2;
constexpr pin_t LED_PINS[LEDS_COUNT]{17, 18};
constexpr pin_t BTN_PIN = 5;
constexpr pin_t BOOT_BTN = 0;
bool btnPressed = false;
bool btnState = false;
bool lastBtnState = LOW;

bool bootBtnPressed = false;
bool bootBtnState = false;
bool lastBootBtnState = HIGH;

void setup() {
  
  for (size_t i = 0; i < LEDS_COUNT; i++)
  {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  pinMode(BTN_PIN, INPUT);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {

  btnState = digitalRead(BTN_PIN);

  if(btnState == HIGH && lastBtnState == LOW) {
 
    btnPressed = !btnPressed;
    Serial.print("btn toggled: ");
    Serial.println(btnState);
  }

  lastBtnState = btnState;

  bootBtnState = digitalRead(BOOT_BTN);
  if(bootBtnState == LOW && lastBootBtnState == HIGH) {
 
    bootBtnPressed = !bootBtnPressed;
    Serial.print("boot toggled: ");
    Serial.println(bootBtnState);
  }

  lastBootBtnState = bootBtnState;

  if(btnPressed) {
 
    policeBlink(LED_PINS[0], LED_PINS[1], 200);
  } else if(bootBtnPressed) {
 
    policeBlink(LED_PINS[0], LED_PINS[1], 500);
  }
    
  

}


void policeBlink(pin_t firstPin, pin_t secondPin, uint16_t milisecondsDelay) {

  digitalWrite(firstPin, HIGH);
  delay(milisecondsDelay);

  digitalWrite(firstPin, LOW);
  digitalWrite(secondPin, HIGH);
  delay(milisecondsDelay);

  digitalWrite(secondPin, LOW);
}
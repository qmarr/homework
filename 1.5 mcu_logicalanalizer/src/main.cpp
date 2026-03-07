#include <Arduino.h>

#define BUTTON_LEFT 15
#define BUTTON_RIGHT 3

int16_t counter_left = 0;
int16_t counter_right = 0;

void IRAM_ATTR reaction_left() {
  counter_left++;
  Serial.println("\nLEFT Button Pressed! Count: " + String(counter_left));
}

void IRAM_ATTR reaction_right() {
  counter_right++;
  Serial.println("\nRIGHT Button Pressed! Count: " + String(counter_right));
}

void setup() {
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT), reaction_left, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT), reaction_right, FALLING);
}

void loop() {
  Serial.print("HELLO");
  delay(250);
}
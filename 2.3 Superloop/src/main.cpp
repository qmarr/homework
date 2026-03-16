#include <Arduino.h>
#include "led.h"

void blinkLed(Led &led)
{
  uint64_t curMillis = millis();
  if (curMillis - led.getLastTick() >= led.getBLinkPeriod())
  {
    led.setLastTick(curMillis);
    led.toggle();
  }
}

Led led1(15);
Led led2(16, 500);
Led led3(17, 1000);

void setup()
{
  led1.init();
  led2.init();
  led3.init();
}

void loop()
{
  blinkLed(led1);
  blinkLed(led2);
  blinkLed(led3);
}

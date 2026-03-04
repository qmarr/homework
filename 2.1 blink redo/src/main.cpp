#include "led.h"
#include "config.h"

void setup()
{
  Serial.begin(115200);
}

void blink(const Config &config, Led &ledToBlink)
{
  uint16_t curMillis = millis();
  static uint16_t lastTick{0};

  if (curMillis - lastTick >= config.getSpeed())
  {
    lastTick = curMillis;
    digitalWrite(ledToBlink.getLedPin(), ledToBlink.makeStateOpposite());
  }
}

Led led1;
const Config conf1;

void loop()
{

  blink(conf1, led1);

  Serial.print("Millis: ");
  Serial.println(millis());
}

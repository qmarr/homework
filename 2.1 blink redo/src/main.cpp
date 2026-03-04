#include "led.h"

void setup()
{
  Serial.begin(115200);
  Led led1;
}

void loop()
{



  Serial.print("Millis: ");
  Serial.println(millis());
}

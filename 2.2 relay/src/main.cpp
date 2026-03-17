#include "relay.h"

constexpr uint8_t RL_PIN{18};

void printState(const Relay &relay)
{
  if (relay.getStatus())
  {
    Serial.println("Relay turned on.");
  }
  else{
    Serial.println("Relay turned off.");
  }
}

Relay relay1(RL_PIN);

void setup()
{
  Serial.begin(115200);
  relay1.init();
}

void loop()
{
  relay1.switchRL();
  printState(relay1);

}

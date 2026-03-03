#include <Arduino.h>

constexpr uint8_t ADC_PIN{15};
constexpr uint8_t ADC_PERIOD{100};
constexpr uint16_t ADC_MAX{4095};
constexpr uint32_t UREF_MV{3300};

void setup()
{
  Serial.begin(115200);
  pinMode(ADC_PIN, INPUT);
}

void loop()
{
  uint16_t adc_value = analogRead(ADC_PIN);
  uint16_t millivolts{analogReadMilliVolts(ADC_PIN)};
  uint32_t volts = (adc_value * UREF_MV) / ADC_MAX;

  Serial.print("RAW: ");
  Serial.println(adc_value);
  Serial.print("Calc Volts: ");
  Serial.println(volts);
  Serial.print("Millivots from method: ");
  Serial.println(millivolts);
  delay(1000);
}

#include <Arduino.h>

constexpr uint8_t ADC_PIN{15};
constexpr uint8_t ADC_PERIOD{100};
constexpr uint16_t ADC_MAX{4095};
constexpr uint32_t UREF_MV{3300}; //can be 3.26 or 3.35

uint32_t calcError(uint32_t calc_mV, uint32_t esp32mV)
{
  if (esp32mV == 0) return 0;

  uint32_t difference = (((calc_mV > esp32mV)) ? calc_mV - esp32mV : (esp32mV - calc_mV));

  return (difference * 100) / calc_mV;
}

void printvalues(uint32_t RAW, uint32_t calc_mV, uint32_t esp32mV)
{
  Serial.print("RAW: \t");
  Serial.println(RAW);
  Serial.print("Calc Volts: \t");
  Serial.print(calc_mV);
  Serial.println(" mV");
  Serial.print("Millivots from method: \t");
  Serial.print(esp32mV);
  Serial.println(" mV");
  Serial.print("Error: ");
  Serial.print(calcError(calc_mV, esp32mV));
  Serial.println("% ");
  delay(2000);
}

void setup()
{
  Serial.begin(115200);
  pinMode(ADC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
}

void loop()
{
  
  uint32_t raw_value = analogRead(ADC_PIN);
  uint32_t millivolts{analogReadMilliVolts(ADC_PIN)};
  uint32_t calc_mvolts = (raw_value * UREF_MV) / ADC_MAX;

  printvalues(raw_value, calc_mvolts, millivolts);
}

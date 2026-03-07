#include <Arduino.h>
constexpr uint8_t LEDS_TO_LIGHT = 5;
constexpr uint8_t LED_PINS[LEDS_TO_LIGHT] = {4, 15, 16, 17, 18};
constexpr uint8_t POT_PIN = 5;
constexpr uint16_t ADC_MAX = 4095;
constexpr uint16_t ADC_STEP = ADC_MAX / LEDS_TO_LIGHT;


uint16_t pot_value = 0;

void setup()
{
    for (size_t i = 0; i < LEDS_TO_LIGHT; i++)
    {
        pinMode(LED_PINS[i], OUTPUT);
    }
    pinMode(POT_PIN, INPUT);
}

void loop()
{
    pot_value = analogRead(POT_PIN);


    uint8_t led_count = ((pot_value + 1) * LEDS_TO_LIGHT) / (ADC_MAX + 1); 
     


    for (size_t i = 0; i < LEDS_TO_LIGHT; i++)
    {

        if (i < led_count )
        {
            digitalWrite(LED_PINS[i], HIGH);
        }
        else
            digitalWrite(LED_PINS[i], LOW);
    }
    
}
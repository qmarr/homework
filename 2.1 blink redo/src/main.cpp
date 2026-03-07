#include "led.h"
#include "config.h"

void setup()
{
  Serial.begin(115200);
}

void blink(const Config &config, Led &ledToBlink)
{
  uint32_t curMillis = millis();
  static uint32_t lastTick{0};

  if (curMillis - lastTick > config.getSpeed())
  {
    lastTick = curMillis;
    digitalWrite(ledToBlink.getLedPin(), ledToBlink.makeStateOpposite());
  }
}

Led led1;
const Config conf1;

void loop()
{

  //  TODO Додати кнопку з перериванням (опційно) 

  //   Під'єднати кнопку до GPIO, налаштувати переривання (attachInterrupt)
  //   Створити volatile bool buttonPressed для сигналізації про натиск
  //   Коли кнопка натиснута, LED змінює режим: блимання → постійно увімкнено → постійно вимкнено → назад до блимання

  //   ISR має бути мінімальною
  //   Не використовувати Serial.print всередині ISR
  //   Логіку перемикання режиму обробляти в loop()

  
  blink(conf1, led1);

  static uint32_t curLoopTick{0};
  static uint32_t lastLoopTick{0};
  if (curLoopTick - lastLoopTick >= 1000)
  {
    lastLoopTick = curLoopTick;
    Serial.print("Millis: ");
    Serial.println(millis());
  }
  curLoopTick++;
}

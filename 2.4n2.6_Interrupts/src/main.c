#include "driver/gpio.h"


#define BTN_GPIO 15

void app_main() {

    gpio_config_t io_conf = {};

    io_conf.pin_bit_mask = (1ULL << BTN_GPIO);
}
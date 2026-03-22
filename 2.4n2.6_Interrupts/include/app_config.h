#ifndef CONFIG_H
#define CONFIG_H

#include <driver/gpio.h>

#define BTN_GPIO            GPIO_NUM_15
#define DEBOUNCE_DELAY_MS   50
#define BTN_QUEUE_LEN       10
#define POLL_MS             10
#define TAG                 "BTN_APP"


typedef enum {
    MODE_RAW_ISR = 1,
    MODE_TIME_DEBOUNCE,
    MODE_STATE_DEBOUNCE,
    MODE_POLLING_FSM
} app_mode_t;

#endif
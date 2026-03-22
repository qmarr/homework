#ifndef BUTTON_H
#define BUTTON_H

#include <freertos/queue.h>
#include <freertos/FreeRTOS.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    BTN_IDLE = 0,
    BTN_BOUNCE_PRESS,
    BTN_PRESSED,
    BTN_BOUNCE_RELEASE

} button_fsm_state_t;

typedef struct
{
    uint64_t timestamp_us;
    int level;
} button_event_t;

void button_init_interrupt_mode(QueueHandle_t queue);
void button_init_polling_mode(void);
int button_read_level(void);

#endif // BUTTON_H
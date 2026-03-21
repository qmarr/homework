#ifndef BUTTON_H
#define BUTTON_H

#include <freertos/queue.h>
#include <freertos/FreeRTOS.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    BUTTON_EVENT_RAW_ISR,
    BUTTON_EVENT_PRESSED
} button_event_type_t;

typedef struct
{
    button_event_type_t type;
    uint64_t timestamp_us;
    int level;
} button_event_t;

void button_init(QueueHandle_t queue);
int button_read_level(void);

#endif // BUTTON_H
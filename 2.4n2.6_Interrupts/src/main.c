#include <stdio.h>

#include <freertos/task.h>
#include <esp_timer.h>
#include "esp_log.h"
#include "sdkconfig.h"

#include "app_config.h"
#include "button.h"

static QueueHandle_t s_button_queue = NULL;

void app_main()
{
    static app_mode_t current_mode = MODE_TIME_DEBOUNCE;

    s_button_queue = xQueueCreate(BTN_QUEUE_LEN, sizeof(button_event_t));
    configASSERT(s_button_queue != NULL);

    button_init(s_button_queue);

    uint32_t raw_interrupt_counter = 0;
    uint64_t last_accepted_time_us = 0;

    button_event_t evt;

    while (1)
    {
        if (xQueueReceive(s_button_queue, &evt, pdMS_TO_TICKS(100)))
        {
            raw_interrupt_counter++;

            // 1st task
            ESP_LOGI(TAG,
                     "RAW ISR count=%" PRIu32 ", ts=%" PRIu64 " us, level=%d",
                     raw_interrupt_counter, evt.timestamp_us, evt.level);

            // 2nd task

            if (evt.timestamp_us - last_accepted_time_us >= (DEBOUNCE_DELAY_US * 1000ULL))
            {
                last_accepted_time_us = evt.timestamp_us;
                ESP_LOGI(TAG, "Accepted after time debounce");
            } else {
                ESP_LOGI(TAG, "Rejected by debounce");
            }

            // 3rd task

            

            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
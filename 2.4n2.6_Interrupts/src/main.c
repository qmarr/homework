#include <stdio.h>

#include <freertos/task.h>
#include <esp_timer.h>
#include <inttypes.h>
#include "esp_log.h"
#include "sdkconfig.h"

#include "app_config.h"
#include "button.h"

static QueueHandle_t s_button_queue = NULL;

void app_main()
{
    static app_mode_t current_mode = MODE_TIME_DEBOUNCE;

    if (current_mode == MODE_POLLING_FSM)
    {
        button_init_polling_mode();
    }
    else
    {
        s_button_queue = xQueueCreate(BTN_QUEUE_LEN, sizeof(button_event_t));
        configASSERT(s_button_queue != NULL);
        button_init_interrupt_mode(s_button_queue);
        
    }

    uint32_t raw_interrupt_counter = 0;
    uint64_t last_accepted_time_us = 0;

    button_fsm_state_t state = BTN_IDLE;
    uint64_t state_enter_time_us = 0;
    uint32_t press_count = 0;

    bool was_pressed = false;
    button_event_t evt;

    while (true)
    {
        if (current_mode != MODE_POLLING_FSM)
        {
            if (xQueueReceive(s_button_queue, &evt, pdMS_TO_TICKS(100)))
            {
                switch (current_mode)
                {
                case MODE_RAW_ISR:
                    raw_interrupt_counter++;
                    // 1st task
                    ESP_LOGI(TAG,
                             "RAW ISR count=%" PRIu32 ", ts=%" PRIu64 " us, level=%d",
                             raw_interrupt_counter, evt.timestamp_us, evt.level);
                    break;
                case MODE_TIME_DEBOUNCE:
                    // 2nd task
                    if (evt.timestamp_us - last_accepted_time_us >= (DEBOUNCE_DELAY_MS * 1000ULL))
                    {
                        last_accepted_time_us = evt.timestamp_us;
                        ESP_LOGI(TAG, "Accepted after time debounce");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Rejected by debounce");
                    }
                    break;

                case MODE_STATE_DEBOUNCE: {
                    uint64_t now = esp_timer_get_time();
                    int current_level = button_read_level();
                    if (current_level == 1)
                    {
                        was_pressed = false;
                    }
                    if (now - last_accepted_time_us >= (DEBOUNCE_DELAY_MS * 1000ULL))
                    {
                        if (current_level == 0 && !was_pressed)
                        {
                            last_accepted_time_us = now;
                            was_pressed = true;
                            ESP_LOGI(TAG, "Button pressed!");
                        }
                    }
                    break;
                }
                }
            }
        }
        if (current_mode == MODE_POLLING_FSM)
        {
            int level = button_read_level();
            uint64_t now = esp_timer_get_time();
            switch (state)
            {
            case BTN_IDLE:
                if (level == 0)
                {
                    state = BTN_BOUNCE_PRESS;
                    state_enter_time_us = now;
                }
                break;
            case BTN_BOUNCE_PRESS:
                if (level == 1)
                {
                    state = BTN_IDLE;
                }
                else if (now - state_enter_time_us >= (DEBOUNCE_DELAY_MS * 1000ULL))
                {
                    state = BTN_PRESSED;
                    press_count++;
                    ESP_LOGI(TAG, "Pressed. Press count = %" PRIu32, press_count);
                }
                break;
            case BTN_PRESSED:
                if (level == 1)
                {
                    state = BTN_BOUNCE_RELEASE;
                    state_enter_time_us = now;
                }
                break;
            case BTN_BOUNCE_RELEASE:
                if (level == 0)
                {
                    state = BTN_PRESSED;
                }
                else if (now - state_enter_time_us >= (DEBOUNCE_DELAY_MS * 1000ULL))
                {
                    state = BTN_IDLE;
                }

                break;
            }
            vTaskDelay(pdMS_TO_TICKS(POLL_MS));
        }
    }
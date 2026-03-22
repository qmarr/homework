#include "button.h"
#include "app_config.h"

#include <driver/gpio.h>
#include "esp_timer.h"
#include "esp_log.h"

static QueueHandle_t button_queue = NULL;

static void IRAM_ATTR button_isr(void *arg)
{
    button_event_t evt = {
        .timestamp_us = esp_timer_get_time(),
        .level = gpio_get_level(BTN_GPIO)};

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button_queue, &evt, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

void button_init_interrupt_mode(QueueHandle_t queue)
{
    button_queue = queue;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTN_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_GPIO, button_isr, NULL));

    ESP_LOGI(TAG, "Button initialized on GPIO %d", BTN_GPIO);
}

void button_init_polling_mode(void)
{
        gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTN_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "Button initialized in polling mode on GPIO %d", BTN_GPIO);
}

int button_read_level(void)
{
    return gpio_get_level(BTN_GPIO);
}
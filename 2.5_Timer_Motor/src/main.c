#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_err.h"

#define TAG "FAN_TIMER"

// ===== DEBUG TIMINGS =====
#define PERIOD_MS   10000   
#define ON_TIME_MS   3000   

#define LED_GPIO GPIO_NUM_18

static esp_timer_handle_t periodic_timer = NULL;
static esp_timer_handle_t stop_timer = NULL;

static bool led_is_on = false;

static void led_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_set_level(LED_GPIO, 0);
}

static void led_set(bool on)
{
    led_is_on = on;
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void stop_timer_callback(void *arg)
{
    if (led_is_on) {
        led_set(false);
        ESP_LOGI(TAG, "LED OFF");
    }
}

static void periodic_timer_callback(void *arg)
{
    if (!led_is_on) {
        led_set(true);
        ESP_LOGI(TAG, "LED ON");

        ESP_ERROR_CHECK(
            esp_timer_start_once(stop_timer, ON_TIME_MS * 1000ULL)
        );
    } else {
        ESP_LOGW(TAG, "Periodic event ignored: LED already ON");
    }
}

void app_main(void)
{
    led_init();

    esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "periodic_timer"
    };

    esp_timer_create_args_t stop_timer_args = {
        .callback = &stop_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "stop_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_create(&stop_timer_args, &stop_timer));

    ESP_ERROR_CHECK(
        esp_timer_start_periodic(periodic_timer, PERIOD_MS * 1000ULL)
    );

    ESP_LOGI(TAG, "Timer control started: period=%d ms, on_time=%d ms",
             PERIOD_MS, ON_TIME_MS);

    while (true) {

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
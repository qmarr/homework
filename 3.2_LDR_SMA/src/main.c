
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define TAG "LDR_APP"

// ===== GPIO =====
#define LED_GPIO        GPIO_NUM_18          
#define LDR_ADC_UNIT    ADC_UNIT_2
#define LDR_ADC_CH      ADC_CHANNEL_4       
#define LDR_ADC_ATTEN   ADC_ATTEN_DB_12

// ===== SMA =====
#define SMA_SIZE 8

// ===== Polling period =====
#define SAMPLE_PERIOD_MS 100

// ===== Hysteresis thresholds =====
#define DARK_THRESHOLD   1200
#define LIGHT_THRESHOLD  1800

static adc_oneshot_unit_handle_t adc_handle;

// SMA buffer
static int sma_buffer[SMA_SIZE] = {0};
static int sma_index = 0;
static int sma_sum = 0;
static bool sma_filled = false;

static bool led_on = false;

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
    led_on = on;
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = LDR_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = LDR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, LDR_ADC_CH, &chan_config));
}

static int adc_read_raw(void)
{
    int raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, LDR_ADC_CH, &raw));
    return raw;
}

static int sma_filter(int new_value)
{
    sma_sum -= sma_buffer[sma_index];
    sma_buffer[sma_index] = new_value;
    sma_sum += new_value;

    sma_index++;
    if (sma_index >= SMA_SIZE) {
        sma_index = 0;
        sma_filled = true;
    }

    int divisor = sma_filled ? SMA_SIZE : sma_index;
    if (divisor == 0) {
        divisor = 1;
    }

    return sma_sum / divisor;
}

static void control_led_with_hysteresis(int filtered_raw)
{
    if (!led_on && filtered_raw < DARK_THRESHOLD) {
        led_set(true);
        ESP_LOGI(TAG, "Dark detected -> LED ON");
    }
    else if (led_on && filtered_raw > LIGHT_THRESHOLD) {
        led_set(false);
        ESP_LOGI(TAG, "Light detected -> LED OFF");
    }
}

void app_main(void)
{
    led_init();
    adc_init();

    ESP_LOGI(TAG, "LDR + SMA + hysteresis started");
    ESP_LOGI(TAG, "dark_threshold=%d, light_threshold=%d", DARK_THRESHOLD, LIGHT_THRESHOLD);

    while (true) {
        int raw = adc_read_raw();
        int filtered = sma_filter(raw);

        control_led_with_hysteresis(filtered);

        ESP_LOGI(TAG, "RAW=%d, SMA=%d, LED=%s",
                 raw, filtered, led_on ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}
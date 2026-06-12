#include <stdio.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define POT_PIN GPIO_NUM_15
#define MOTOR_GPIO GPIO_NUM_18
#define LED_GPIO GPIO_NUM_4
#define POT_ADC_UNIT ADC_UNIT_2
#define POT_CHANNEL ADC_CHANNEL_4
#define POT_ATTEN ADC_ATTEN_DB_12
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY ((1 << PWM_RESOLUTION) - 1)
#define PWM_FREQ 1000
#define UREF_MV 3300
#define ADC_BITS 12
#define ADC_MAX ((1UL << ADC_BITS) - 1)
#define MOTOR_MIN_DUTY 500

adc_oneshot_unit_handle_t adc_handle1;

void pwm_init()
{
    ledc_timer_config_t timer_cfg = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_0,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t led_cfg = {
        .gpio_num = LED_GPIO,
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&led_cfg));

    ledc_channel_config_t motor_cfg = {
        .gpio_num = MOTOR_GPIO,
        .channel = LEDC_CHANNEL_1,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&motor_cfg));
}

static void pot_init()
{
    adc_oneshot_unit_init_cfg_t adc_conf = {
        .unit_id = POT_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_conf, &adc_handle1));

    adc_oneshot_chan_cfg_t adc_chan_cfg = {
        .atten = POT_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle1, POT_CHANNEL, &adc_chan_cfg));
}

void app_main()
{
    pot_init();
    pwm_init();

    int raw = 0,
        duty = 0,
        motor_duty = 0;

    while (true)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle1, POT_CHANNEL, &raw));

        duty = (raw * PWM_MAX_DUTY) / ADC_MAX;

        if (duty < MOTOR_MIN_DUTY)
        {
            motor_duty = 0;
        }
        else
        {
            motor_duty = duty;
        }
        printf("RAW=%d duty=%d motor_duty = %d\n", raw, duty, motor_duty);

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, motor_duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
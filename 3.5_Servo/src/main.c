#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "example";

// Please consult the datasheet of your servo before changing the following parameters
#define SERVO_MIN_PULSEWIDTH_US 500  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2500 // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE 0           // Minimum angle
#define SERVO_MAX_DEGREE 180         // Maximum angle

#define SERVO_PULSE_GPIO 18                  // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000          // 20000 ticks, 20ms

#define POT_PIN GPIO_NUM_15
#define POT_ADC_UNIT ADC_UNIT_2
#define POT_CHANNEL ADC_CHANNEL_4
#define POT_ATTEN ADC_ATTEN_DB_12

#define ADC_BITS 12
#define ADC_MAX ((1 << ADC_BITS) - 1)

#define POT_FULL_DEGREE 270
#define SERVO_FULL_DEGREE 180

#define ADC_USED_MAX ((ADC_MAX * SERVO_FULL_DEGREE) / POT_FULL_DEGREE)

static inline uint32_t servo_angle_to_pulse_us(uint32_t angle)
{
    return SERVO_MIN_PULSEWIDTH_US +
           angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

adc_oneshot_unit_handle_t adc_handle1;

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
    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, 
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SERVO_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, 1500));

    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    int raw = 0;
    int pulse = 0;
    int servo_angle_from_left = 0;

    while (1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle1, POT_CHANNEL, &raw));

        if (raw > ADC_USED_MAX)
        {
            raw = ADC_USED_MAX;
        }

        servo_angle_from_left = raw * SERVO_FULL_DEGREE / ADC_USED_MAX;
        pulse = servo_angle_to_pulse_us(servo_angle_from_left);

        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, pulse));
        ESP_LOGI(TAG, "Angle from left: %d deg, pulse: %d us",
                 servo_angle_from_left, pulse);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
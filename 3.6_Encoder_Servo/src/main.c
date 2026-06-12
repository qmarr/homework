#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

static const char *TAG = "Servo-Encoder";


#define SERVO_MIN_PULSEWIDTH_US 500 // Minimum pulse width in microsecond
#define SERVO_MID_PULSEWIDTH_US 1500
#define SERVO_MAX_PULSEWIDTH_US 2500 // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE 0           // Minimum angle
#define SERVO_MAX_DEGREE 180         // Maximum angle

#define SERVO_PULSE_GPIO 18                  // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000          // 20000 ticks, 20ms

#define EXAMPLE_PCNT_HIGH_LIMIT 70
#define EXAMPLE_PCNT_LOW_LIMIT -70
#define STEP_DEGREE_SMALL 4
#define STEP_DEGREE_BIG 30

#define ATTEN ADC_ATTEN_DB_12
#define ADC_BITS 12
#define ADC_MAX ((1 << ADC_BITS) - 1)

#define SERVO_FULL_DEGREE 180
#define BTN_LONG_PRESS 3000

#define CHAN_GPIO_A GPIO_NUM_16
#define CHAN_GPIO_B GPIO_NUM_17
#define BTN_GPIO_ENC GPIO_NUM_4
#define BUZZER_GPIO GPIO_NUM_21

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define PWM_RES_BITS 10
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY ((1 << PWM_RES_BITS) - 1)
#define PWM_FREQ 440

static inline uint32_t servo_angle_to_pulse_us(uint32_t angle)
{
    return SERVO_MIN_PULSEWIDTH_US +
           angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE);
}

static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

uint32_t duty = PWM_MAX_DUTY / 2;

void buzzer_init()
{
    ledc_timer_config_t timer_cfg = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_0,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t buzzer_cfg = {
        .gpio_num = BUZZER_GPIO,
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&buzzer_cfg));
}

void buzzer_start(uint32_t freq)
{
    ledc_set_freq(LEDC_MODE, LEDC_TIMER_0, freq);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));
}

void buzzer_stop()
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));
}

void beep()
{
    buzzer_start(440);
    vTaskDelay(pdMS_TO_TICKS(70));
    buzzer_stop();
}

pcnt_unit_handle_t pcnt_unit = NULL;
pcnt_channel_handle_t pcnt_chan = NULL;

static void encoder_init()
{

    pcnt_unit_config_t unit_config = {
        .high_limit = EXAMPLE_PCNT_HIGH_LIMIT,
        .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
    };

    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    ESP_LOGI(TAG, "install pcnt BUTTON");
    gpio_config_t button_gpio_config = {
        .pin_bit_mask = (1ULL << BTN_GPIO_ENC),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&button_gpio_config));

    ESP_LOGI(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = CHAN_GPIO_A,
        .level_gpio_num = CHAN_GPIO_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = CHAN_GPIO_B,
        .level_gpio_num = CHAN_GPIO_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
}

mcpwm_timer_handle_t timer = NULL;
mcpwm_oper_handle_t oper = NULL;
mcpwm_cmpr_handle_t comparator = NULL;
mcpwm_gen_handle_t generator = NULL;

static void mcpw_init()
{
    ESP_LOGI(TAG, "Create timer and operator");

    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");

    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SERVO_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, SERVO_MID_PULSEWIDTH_US));

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
}

void app_main()
{

    encoder_init();
    mcpw_init();
    buzzer_init();

    ESP_LOGI(TAG, "add watch points and register callbacks");
    int watch_points[] = {EXAMPLE_PCNT_LOW_LIMIT, -12, 0, 12, EXAMPLE_PCNT_HIGH_LIMIT};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++)
    {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
    }
    pcnt_event_callbacks_t cbs = {
        .on_reach = example_pcnt_on_reach,
    };
    QueueHandle_t queue = xQueueCreate(10, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));


    int pulse_count = 0;
    int last_count = 0;
    int angle = 90;
    int delta = 0;
    bool fine_mode = false;

    int last_btn_level = 1;
    int stable_btn_level = 1;
    int64_t last_btn_change_us = 0;

    int64_t btn_press_start_us = 0;
    bool long_press_handled = false;
    while (1)
    {

        // button
        int btn_level = gpio_get_level(BTN_GPIO_ENC);
        int64_t now_us = esp_timer_get_time();

        if (btn_level != last_btn_level)
        {
            last_btn_level = btn_level;
            last_btn_change_us = now_us;
        }

        // debounce
        if ((now_us - last_btn_change_us) > 50000) // 50ms
        {
            if (btn_level != stable_btn_level)
            {
                stable_btn_level = btn_level;

                if (stable_btn_level == 0)
                {
                    btn_press_start_us = now_us;
                    long_press_handled = false;
                }
                else
                {

                    if (!long_press_handled)
                    {
                        fine_mode = !fine_mode;
                        ESP_LOGI(TAG, "Short button press: fine=%d", fine_mode);
                    }
                }
            }
        }
        if (stable_btn_level == 0 &&
            !long_press_handled &&
            (now_us - btn_press_start_us) > BTN_LONG_PRESS * 1000ULL)
        {
            angle = 90;
            ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(
                comparator,
                servo_angle_to_pulse_us(angle)));

            long_press_handled = true;

            ESP_LOGI(TAG, "LONG BUTTON PRESS -> center");
        }

        // encoder
        ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
        delta = pulse_count - last_count;
        if (delta != 0)
        {
            ESP_LOGI(TAG, "Encoder count:\t %d, \tdelta: %d",
                     pulse_count, pulse_count - last_count);
        }
        last_count = pulse_count;

        // step
        int step = fine_mode ? STEP_DEGREE_SMALL : STEP_DEGREE_BIG;

        // update servo
        if (delta != 0)
        {
            int new_angle = angle + delta * step;

            if (new_angle < 0)
            {
                new_angle = 0;
                beep();
            }
            else if (new_angle > 180)
            {
                new_angle = 180;
                beep();
            }
            angle = new_angle;

            ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, servo_angle_to_pulse_us(new_angle)));

            ESP_LOGI(TAG, "Angle: \t\t%d, \tstep: %d, fine: %d",
                     angle, step, fine_mode);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_err.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#define BUZZER_GPIO GPIO_NUM_4
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define PWM_RES_BITS 10
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY ((1 << PWM_RES_BITS) - 1)
#define PWM_FREQ 440

#define PLAYER_TICK_MS 50

esp_timer_handle_t periodic_timer = NULL;

bool is_playing = false;
uint32_t duty = PWM_MAX_DUTY / 2;

static uint32_t note_index = 0;
static uint32_t remaining_ms = 0;
static bool player_active = false;

typedef struct
{
    uint32_t freq_hz;
    uint32_t duration_ms;
} note_t;


static const note_t melody[] = {
    {392, 300},  // G4
    {0,   100},
    {440, 300},  // A4
    {0,   100},
    {523, 500},  // C5
    {0,   200},

    {494, 300},  // B4
    {0,   100},
    {440, 300},  // A4
    {0,   100},
    {392, 600},  // G4
    {0,   300},

    {330, 300},  // E4
    {0,   100},
    {392, 300},  // G4
    {0,   100},
    {440, 700},  // A4
    {0,   300},

    {392, 300},  // G4
    {0,   100},
    {349, 300},  // F4
    {0,   100},
    {330, 800},  // E4
};

#define MELODY_LEN (sizeof(melody) / sizeof(melody[0]))

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
    is_playing = true;
}

void buzzer_stop()
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));
    is_playing = false;
}

void start_current_note()
{
    note_t note = melody[note_index];

    remaining_ms = note.duration_ms;
    if (note.freq_hz == 0)
    {
        buzzer_stop();
    }
    else
    {
        buzzer_start(note.freq_hz);
    }
}

void player_start()
{
    note_index = 0;
    player_active = true;
    start_current_note();
}

void player_tick(void *arg)
{
    if (!player_active)
    {
        return;
    }

    if (remaining_ms > PLAYER_TICK_MS)
    {
        remaining_ms -= PLAYER_TICK_MS;
        return;
    }

    note_index++;

    if (note_index >= MELODY_LEN)
    {
        buzzer_stop();
        player_active = false;
        return;
    }

    start_current_note();
}

void timer_init()
{

    esp_timer_create_args_t periodic_timer_args = {
        .callback = &player_tick,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "periodic_timer"};

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
}

void app_main()
{
    buzzer_init();
    timer_init();

    player_start();

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, PLAYER_TICK_MS * 1000ULL));

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
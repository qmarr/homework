#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"

#include "font5x7.h"
#include "ssd1306.h"
#include "ds1307.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_9
#define I2C_MASTER_SDA_IO GPIO_NUM_8
#define I2C_MASTER_NUM I2C_NUM_0 // I2C порт 0
#define I2C_MASTER_FREQ_HZ 100000
#define OLED_ADDR 0x3C

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t oled_dev_handle;
static i2c_master_dev_handle_t clock_dev_handle;

void i2c_init()
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t oled_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &oled_dev_cfg, &oled_dev_handle));
}

const char *weekday_to_str(uint8_t dow)
{
    static const char *names[] = {
        "???",
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
    };

    if (dow > 7) {
        return names[0];
    }

    return names[dow];
}

void display_update_time(i2c_master_dev_handle_t oled_handle, const ds1307_datetime_t *dt)
{
    char line1[22];
    char line2[22];

    snprintf(line1, sizeof(line1),
             "%02u:%02u:%02u",
             dt->hour, dt->min, dt->sec);

    snprintf(line2, sizeof(line2),
             "%s %02u.%02u.%04u",
             weekday_to_str(dt->day_of_week),
             dt->date, dt->month, dt->year);

    ssd1306_clear_line(oled_handle, 1);
    ssd1306_clear_line(oled_handle, 2);

    ssd1306_print_at(oled_handle, 1, 0, line1);
    ssd1306_print_at(oled_handle, 2, 0, line2);
}

void app_main()
{
    i2c_init();
    ssd1306_init(oled_dev_handle);
    ssd1306_clear(oled_dev_handle);

    ssd1306_print_at(oled_dev_handle, 0, 0, "Salut, Zooble!");

    ESP_ERROR_CHECK(ds1307_init(bus_handle, &clock_dev_handle));

    uint64_t last_update_us = 0;

    while (1)
    {
        uint64_t now_us = esp_timer_get_time();

        if (now_us - last_update_us >= 1000000ULL)
        {
            last_update_us = now_us;

            ds1307_datetime_t dt;

            esp_err_t err = ds1307_read_time(clock_dev_handle, &dt);
            if (err == ESP_OK)
            {
                display_update_time(oled_dev_handle, &dt);

                ESP_LOGI("APP", "%02u:%02u:%02u %s %02u.%02u.%04u",
                         dt.hour, dt.min, dt.sec,
                         weekday_to_str(dt.day_of_week),
                         dt.date, dt.month, dt.year);
            }
            else
            {
                ESP_LOGE("APP", "Failed to read DS1307: %s", esp_err_to_name(err));
                ssd1306_clear_line(oled_dev_handle, 0);
                ssd1306_print_at(oled_dev_handle, 0, 0, "RTC ERROR");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
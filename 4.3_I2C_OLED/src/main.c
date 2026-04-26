#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "font5x7.h"
#include "ssd1306.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_9
#define I2C_MASTER_SDA_IO GPIO_NUM_8
#define I2C_MASTER_NUM I2C_NUM_0 // I2C порт 0
#define I2C_MASTER_FREQ_HZ 100000
#define OLED_ADDR 0x3C

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;


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

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
}
void app_main()
{
    i2c_init();
    ssd1306_init(dev_handle);
    ssd1306_clear(dev_handle);

    ssd1306_print_at(dev_handle, 3, 20, "Salut, Zooble!");

    while (1)
    {

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
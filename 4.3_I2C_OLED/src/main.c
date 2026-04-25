#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_9
#define I2C_MASTER_SDA_IO GPIO_NUM_8
#define I2C_MASTER_NUM I2C_NUM_0 // I2C порт 0
#define I2C_MASTER_FREQ_HZ 100000
#define OLED_ADDR 0x3C

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;

static esp_err_t ssd1306_cmd(i2c_master_dev_handle_t dev, uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    return i2c_master_transmit(dev, buf, sizeof(buf), -1);
}
static esp_err_t ssd1306_cmd2(i2c_master_dev_handle_t dev, uint8_t cmd, uint8_t arg)
{
    uint8_t buf[3] = {0x00, cmd, arg};
    return i2c_master_transmit(dev, buf, sizeof(buf), -1);
}

static esp_err_t ssd1306_data(i2c_master_dev_handle_t dev, const uint8_t *data, size_t len)
{
    uint8_t buf[129];
    if (len > 129)
    {
        return ESP_ERR_INVALID_SIZE;
    }

    buf[0] = 0x40;

    for (size_t i = 0; i < len; i++)
    {
        buf[i + 1] = data[i];
    }

    return i2c_master_transmit(dev, buf, len + 1, -1);
}

static void ssd1306_init(i2c_master_dev_handle_t dev)
{
    ssd1306_cmd(dev, 0xAE); // Display OFF

    ssd1306_cmd2(dev, 0xD5, 0x80); // Clock divide
    ssd1306_cmd2(dev, 0xA8, 0x3F); // Multiplex ratio 1/64
    ssd1306_cmd2(dev, 0xD3, 0x00); // Display offset
    ssd1306_cmd(dev, 0x40);        // Display start line

    ssd1306_cmd2(dev, 0x8D, 0x14); // Charge pump ON
    ssd1306_cmd2(dev, 0x20, 0x00); // Horizontal addressing mode

    ssd1306_cmd(dev, 0xA1); // Segment remap
    ssd1306_cmd(dev, 0xC8); // COM scan direction remapped

    ssd1306_cmd2(dev, 0xDA, 0x12); // COM pins config for 128x64
    ssd1306_cmd2(dev, 0x81, 0x7F); // Contrast

    ssd1306_cmd2(dev, 0xD9, 0xF1); // Pre-charge
    ssd1306_cmd2(dev, 0xDB, 0x40); // VCOMH deselect

    ssd1306_cmd(dev, 0xA4); // Resume RAM content
    ssd1306_cmd(dev, 0xA6); // Normal display
    ssd1306_cmd(dev, 0xAF); // Display ON
}

static void ssd1306_set_full_area(i2c_master_dev_handle_t dev)
{
    ssd1306_cmd(dev, 0x21); // Set column address
    ssd1306_cmd(dev, 0);    // Start column
    ssd1306_cmd(dev, 127);  // End column

    ssd1306_cmd(dev, 0x22); // Set page address
    ssd1306_cmd(dev, 0);    // Start page
    ssd1306_cmd(dev, 7);    // End page
}

static void ssd1306_clear(i2c_master_dev_handle_t dev)
{
    uint8_t zeros[128] = {0};

    ssd1306_set_full_area(dev);

    for (int page = 0; page < 8; ++page)
    {
        ESP_ERROR_CHECK(ssd1306_data(dev, zeros, sizeof(zeros)));
    }
}

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

    uint8_t line[128];
    for (int i = 0; i < 128; ++i)
    {
        line[i] = 0xFF;
    }
    ssd1306_set_full_area(dev_handle);

    for (int page = 0; page < 8; ++page)
    {
        ssd1306_data(dev_handle, line, 128);
    }

    while (1)
    {

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
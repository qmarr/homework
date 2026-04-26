#include "ssd1306.h"

esp_err_t ssd1306_cmd(i2c_master_dev_handle_t dev, uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    return i2c_master_transmit(dev, buf, sizeof(buf), -1);
}
esp_err_t ssd1306_cmd2(i2c_master_dev_handle_t dev, uint8_t cmd, uint8_t arg)
{
    uint8_t buf[3] = {0x00, cmd, arg};
    return i2c_master_transmit(dev, buf, sizeof(buf), -1);
}

esp_err_t ssd1306_data(i2c_master_dev_handle_t dev, const uint8_t *data, size_t len)
{
    uint8_t buf[129];
    if (len > 128)
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

void ssd1306_init(i2c_master_dev_handle_t dev)
{
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xAE)); // Display OFF

    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0xD5, 0x80)); // Clock divide
    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0xA8, 0x3F)); // Multiplex ratio 1/64
    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0xD3, 0x00)); // Display offset
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0x40));        // Display start line

    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0x8D, 0x14)); // Charge pump ON
    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0x20, 0x00)); // Horizontal addressing mode

    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xA1)); // Segment remap
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xC8)); // COM scan direction remapped

    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0xDA, 0x12)); // COM pins config for 128x64
    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0x81, 0x7F)); // Contrast

    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0xD9, 0xF1)); // Pre-charge
    ESP_ERROR_CHECK(ssd1306_cmd2(dev, 0xDB, 0x40)); // VCOMH deselect

    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xA4)); // Resume RAM content
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xA6)); // Normal display
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xAF)); // Display ON
}

void ssd1306_set_full_area(i2c_master_dev_handle_t dev)
{
    ssd1306_cmd(dev, 0x21); // Set column address
    ssd1306_cmd(dev, 0);    // Start column
    ssd1306_cmd(dev, 127);  // End column

    ssd1306_cmd(dev, 0x22); // Set page address
    ssd1306_cmd(dev, 0);    // Start page
    ssd1306_cmd(dev, 7);    // End page
}

void ssd1306_clear(i2c_master_dev_handle_t dev)
{
    uint8_t zeros[128] = {0};

    ssd1306_set_full_area(dev);

    for (int page = 0; page < 8; ++page)
    {
        ESP_ERROR_CHECK(ssd1306_data(dev, zeros, sizeof(zeros)));
    }
}

const uint8_t *get_char_bitmap(char c)
{
    if (c < 32 || c > 127)
    {
        c = ' ';
    }

    return font5x7[c - 32];
}

void ssd1306_draw_char(i2c_master_dev_handle_t dev, char c)
{
    const uint8_t *bitmap = get_char_bitmap(c);

    ESP_ERROR_CHECK(ssd1306_data(dev, bitmap, 5));

    uint8_t spacing = 0x00;
    ESP_ERROR_CHECK(ssd1306_data(dev, &spacing, 1));
}

void ssd1306_draw_string(i2c_master_dev_handle_t dev, const char *str)
{

    while (*str)
    {
        ssd1306_draw_char(dev, *str);
        str++;
    }
}

void ssd1306_set_cursor(i2c_master_dev_handle_t dev, uint8_t page, uint8_t col)
{
    if (page > 7)
    {
        page = 7;
    }
    if (col > 127)
    {
        col = 127;
    }

    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0xB0 | page));                // page 0..7
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0x00 | (col & 0x0F)));        // lower column
    ESP_ERROR_CHECK(ssd1306_cmd(dev, 0x10 | ((col >> 4) & 0x0F))); // higher column
}

void ssd1306_print_at(i2c_master_dev_handle_t dev, uint8_t page, uint8_t col, const char *text)
{
    ssd1306_set_cursor(dev, page, col);
    ssd1306_draw_string(dev, text);
}

void ssd1306_clear_line(i2c_master_dev_handle_t dev, uint8_t page)
{
    uint8_t zeros[128] = {0x00};

    ssd1306_set_cursor(dev, page, 0);
    ESP_ERROR_CHECK(ssd1306_data(dev, zeros, sizeof(zeros)));
}
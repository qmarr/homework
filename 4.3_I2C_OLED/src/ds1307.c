#include "ds1307.h"

#define DS1307_ADDR 0x68
#define DS1307_REG_SECONDS 0x00

static uint8_t bcd_to_dec(uint8_t bcd)
{
    return ((bcd >> 4) * 10 + (bcd & 0x0F));
}

static uint8_t dec_to_bcd(uint8_t dec)
{

    return ((dec / 10) << 4) | (dec % 10);
}

esp_err_t ds1307_init(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t *rtc_handle)
{
    i2c_device_config_t rtc_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DS1307_ADDR,
        .scl_speed_hz = 100000,
    };

    return i2c_master_bus_add_device(bus, &rtc_dev_cfg, rtc_handle);
}
esp_err_t ds1307_read_time(i2c_master_dev_handle_t rtc_handle, ds1307_datetime_t *dt)
{
    uint8_t reg = DS1307_REG_SECONDS;
    uint8_t data[7] = {0};

    esp_err_t err = i2c_master_transmit_receive(
        rtc_handle,
        &reg,
        1,
        data,
        sizeof(data),
        -1);

    if (err != ESP_OK)
    {
        return err;
    }

    // data[0] seconds: bit7 is CH flag, mask it out
    dt->sec = bcd_to_dec(data[0] & 0x07F);
    dt->min = bcd_to_dec(data[1] & 0x07F);
    // 24-hour mode
    dt->hour = bcd_to_dec(data[2] & 0x3F);

    dt->day_of_week = bcd_to_dec(data[3] & 0x07);
    dt->date = bcd_to_dec(data[4] & 0x3F);
    dt->month = bcd_to_dec(data[5] & 0x1F);
    dt->year = 2000 + bcd_to_dec(data[6]);

    return ESP_OK;
}


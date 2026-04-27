#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;

    uint8_t day_of_week; // 1..7
    uint8_t date;        // 1..31
    uint8_t month;       // 1..12
    uint16_t year;       // 2000..2099
} ds1307_datetime_t;

esp_err_t ds1307_init(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t *rtc_handle);
esp_err_t ds1307_read_time(i2c_master_dev_handle_t rtc_handle, ds1307_datetime_t *dt);
#endif
#include <stdio.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_cali.h>
#include <esp_err.h>
#include <soc/adc_channel.h>
#include <hal/adc_types.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define POT_PIN GPIO_NUM_15
#define POT_ADC_UNIT ADC_UNIT_2
#define POT_CHANNEL ADC_CHANNEL_4
#define PIT_ATTEN ADC_ATTEN_DB_6
#define UREF_MV 3300
#define ADC_BITS 12
#define ADC_MAX ((1UL << ADC_BITS) - 1)

void app_main()
{

    adc_oneshot_unit_handle_t adc_handle1;
    adc_oneshot_unit_init_cfg_t adc_conf = {
        .unit_id = POT_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_conf, &adc_handle1));

    adc_oneshot_chan_cfg_t adc_chan_cfg = {
        .atten = PIT_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle1, POT_CHANNEL, &adc_chan_cfg));

    adc_cali_handle_t cali_handle = NULL;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = POT_ADC_UNIT,
        .atten = PIT_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
    int raw = 0,
        u_manual_mv = 0,
        u_cali_mv = 0;
    float error = 0.0f;

    printf("RAW\tU_manual(mV)\tU_cali(mV)\tError(%%)\n");
    printf("-----------------------------------------------\n");
    while (true)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle1, POT_CHANNEL, &raw));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &u_cali_mv));

        u_manual_mv = (raw * UREF_MV) / ADC_MAX;

        int diff = u_manual_mv - u_cali_mv;
        if (diff < 0)
            diff = -diff;
        if (u_cali_mv > 0)
        {
            error = (diff * 100.0f) / u_cali_mv;
        }
        else
        {
            error = 0;
        }

        printf("%d\t%d\t\t%d\t\t%.2f\n", raw, u_manual_mv, u_cali_mv, error);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
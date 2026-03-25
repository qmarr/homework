#include <stdio.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_cali.h>
#include <esp_err.h>

#define POT_PIN GPIO_NUM_15
#define UREF_MV 3300

void app_main()
{

    adc_oneshot_unit_handle_t adc_handle1;
    adc_oneshot_unit_init_cfg_t adc_conf = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_conf, &adc_handle1));

    adc_oneshot_chan_cfg_t adc_chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle1, EXAMPLE_ADC1_CHAN0, &adc_chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle1, EXAMPLE_ADC1_CHAN1, &adc_chan_cfg));
    uint16_t RAW, mV, cl_mV, error;
    while (true)
    {

        printf("RAW\tU_manual(mV)\tU_cali(mV)\tError(%%)\n");
        printf("-----------------------------------------------\n");
        printf("%d\t%.1f\t\t%d\t\t%.2f\n", RAW, mV, cl_mV, error);
    }
}
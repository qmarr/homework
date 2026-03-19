#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include "sdkconfig.h"

#define BTN_GPIO GPIO_NUM_15
#define DEBOUNCE_DELAY_US 200000ULL

static volatile last_isr_time = 0;
static volatile uint32_t counter = 0;

static QueueHandle_t button_queue;

static void IRAM_ATTR button_isr(void *arg) {
    uint64_t now =  esp_timer_get_time();

    if(now - last_isr_time >= DEBOUNCE_DELAY_US) {
        counter++;
        uint32_t cnt = counter;
        BaseType_t higher_priority_task_woken = pdFALSE;
        xQueueSendFromISR(button_queue, &cnt, &higher_priority_task_woken);
        last_isr_time = now;
        if(higher_priority_task_woken) 
            portYIELD_FROM_ISR();
    }
}

void app_main() {

    button_queue = xQueueCreate(10, sizeof(uint32_t));



    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLDOWN_ENABLE
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BTN_GPIO, button_isr, NULL);

    uint32_t button_counter;

    while(1) {
        if(xQueueReceive(button_queue, &button_counter, portMAX_DELAY)) {
            printf("Button pressed %lu times.\n", button_counter);
        }
    }
}
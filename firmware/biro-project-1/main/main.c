#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2  // Change this to your actual LED GPIO pin

void blink_task(void *pvParameter)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));  // 500ms ON
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));  // 500ms OFF
    }
}

void app_main(void)
{
    printf("ESP32 LED Blink Example!\n");
    xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);
}



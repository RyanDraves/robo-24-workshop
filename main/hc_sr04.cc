#include "hc_sr04.hpp"


#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#define NOP asm volatile("nop")


// `IRAM_ATTR` says "please put this function in IRAM" which is faster to access
// than flash memory when code is normally executed-in-place from flash.
// This is useful for a function that needs precise timing.
void IRAM_ATTR delay_microseconds(uint32_t us) {
    uint32_t start = esp_timer_get_time();
    if (us) {
        uint32_t end = start + us;
        while (esp_timer_get_time() < end) {
            NOP;
        }
    }
}


HcSr04::HcSr04(gpio_num_t trigger_gpio, gpio_num_t echo_gpio)
    : trigger_(trigger_gpio), echo_(echo_gpio) {
    gpio_config_t io_conf;

    // Configure the trigger pin as an output
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << trigger_;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    // Set the trigger pin low
    gpio_set_level(trigger_, 0);

    // Configure the echo pin as an input
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1 << echo_;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

HcSr04::~HcSr04() {}

uint32_t HcSr04::get_distance_mm() {
    // Trigger the sensor
    gpio_set_level(trigger_, 1);
    delay_microseconds(10);
    gpio_set_level(trigger_, 0);

    // Wait for the echo to go high for up to kMaxDistanceUs
    uint32_t timeout_us = esp_timer_get_time() + kMaxDistanceUs;
    while (!gpio_get_level(echo_)) {
        if (esp_timer_get_time() > timeout_us) {
            return 0;
        }
    }

    // Start the timer
    uint32_t echo_start_us = esp_timer_get_time();
    timeout_us = echo_start_us + kMaxDistanceUs;

    // Wait for the echo to go low for up to kMaxDistanceUs
    while (gpio_get_level(echo_)) {
        if (esp_timer_get_time() > timeout_us) {
            // Assume the result is max range if the echo pin is still high
            break;
        }
    }

    // Stop the timer
    uint32_t echo_end_us = esp_timer_get_time();

    // Calculate the distance
    uint32_t pulse_width_us = echo_end_us - echo_start_us;
    // Calculated from the assumed speed of sound in air at sea level (~340
    // m/s); pulse. Constant provided in the datasheet.
    uint32_t distance_mm = (uint64_t)(pulse_width_us * 1000) / 5800;

    return distance_mm;
}


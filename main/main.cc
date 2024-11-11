/*
 * Robo 24 Workshop project
 *
 * See `README.md` for install & usage instructions.
 * 
 * Primary task: update the `handle_measurement_request` function to safely
 * handle the rate limit of the HC-SR04 sensor.
 * 
 * Bonus task: add a unit test & hardware integration test for the `HcSr04` class.
 * 
 * Bonus task: write a calibration script to empirically find the rate limit
 * of the HC-SR04 sensor.
 */

#include <iostream>
#include <inttypes.h>
#include "driver/uart.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "tusb_console.h"

#include "hc_sr04.hpp"
#include "json.hpp"

using json = nlohmann::json;

extern "C" {
    void app_main(void);
}

struct MeasurementRequest {
    uint8_t pretty_please;
};

struct Measurement {
    uint32_t distance_mm;
    uint32_t timestamp_ms;
};

/*
 * Handler for a measurement request.
 *
 * The existing implementation naively services the request without considering
 * the rate limit of the HC-SR04 sensor.
 */
void handle_measurement_request(HcSr04& sensor, Measurement *meas) {
    meas->distance_mm = sensor.get_distance_mm();
    // Gets the current time in microseconds and converts it to milliseconds
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html#obtaining-current-time
    meas->timestamp_ms = esp_timer_get_time() / 1000;
}

void app_main(void) {
    // Initialize some "globals"
    HcSr04 sensor((gpio_num_t)1 /* trigger */, (gpio_num_t)2  /* echo */);
    MeasurementRequest req;
    Measurement meas;
    char *buf = (char *)malloc(1024);

    while (true) {
        // TODO: Get stdin working; nothing is being read from stdin
        // Check stdin for a request
        // auto size = fread(buf, 1, 3, stdin);
        // if (size > 0) {
        //     std::cout << "Received request" << std::endl;
        //     std::cout << buf << std::endl;
        // }
        // int len = uart_read_bytes(UART_NUM_0, (uint8_t *)buf, sizeof(buf), 100 / portTICK_PERIOD_MS);
        // std::cout << "Received request: " << len << std::endl;

        handle_measurement_request(sensor, &meas);

        // Serialize the measurement and send it to stdout
        json j;
        j["distance"] = meas.distance_mm;
        std::cout << j.dump() << std::endl;

        // Arbitrary delay
        // TODO: Remove me when requests are being read from stdin
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    std::cout << "Unexpected exit" << std::endl;
    fflush(stdout);
    esp_restart();
}

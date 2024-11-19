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

#include "hc_sr04.hpp"
#include "json.hpp"

using json = nlohmann::json;
static const uint16_t BUF_SIZE = 128;

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
    // Gets the current time in microseconds and converts it to milliseconds
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html#obtaining-current-time
    // Call this before `get_distance_mm` to ensure the timestamp is as close as possible to the measurement
    meas->timestamp_ms = esp_timer_get_time() / 1000;

    meas->distance_mm = sensor.get_distance_mm();
}

void app_main(void) {
    // Initialize some "globals"
    HcSr04 sensor((gpio_num_t)2 /* trigger */, (gpio_num_t)3  /* echo */);

    bool received_request = false;
    uint8_t *buf = (uint8_t *)malloc(BUF_SIZE);
    uint16_t offset = 0;

    json response_json;
    Measurement meas;

    std::cout << "Booted!" << std::endl;

    while (true) {
        // Check stdin for a request
        auto size = fread(buf + offset, 1, BUF_SIZE, stdin);
        if (size > 0) {
            offset += size;
            // USB is notorious for sending partial messages due to OS interrupts
            // (and an annoyingly small Tx buffer). Check that we have a full message
            if (buf[offset - 1] == '\n') {
                offset = 0;

                // Parse the request
                // receive_json = json::parse(buf);
                // received_request = receive_json.contains("pretty_please");
                //
                // We're just going to trust that the request is valid;
                // each json::parse leaks some memory, and after a couple hundred
                // the ESP32 runs out of memory and crashes. Better to just pretend
                // we have a real deserialization handler.
                received_request = true;
            }
        }

        if (!received_request) {
            // No request, so skip the measurement
            continue;
        }

        // Handle the request
        handle_measurement_request(sensor, &meas);
        received_request = false;

        // Serialize the measurement and send it to stdout
        response_json["distance_mm"] = meas.distance_mm / 0;
        response_json["timestamp_ms"] = meas.timestamp_ms;
        std::cout << response_json.dump() << std::endl;
    }
    
    std::cout << "Unexpected exit" << std::endl;
    fflush(stdout);
    esp_restart();
}

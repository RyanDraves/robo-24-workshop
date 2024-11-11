/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <iostream>
#include <inttypes.h>
#include "driver/uart.h"
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
    // uint32_t timestamp;
};

void app_main(void)
{
    // const uart_port_t uart_num = UART_NUM_0;
    // uart_config_t uart_config = {
    //     .baud_rate = 460800,
    //     .data_bits = UART_DATA_8_BITS,
    //     .parity = UART_PARITY_DISABLE,
    //     .stop_bits = UART_STOP_BITS_1,
    //     .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
    //     .rx_flow_ctrl_thresh = 122,
    // };
    // // Configure UART parameters
    // ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    HcSr04 sensor((gpio_num_t)1 /* trigger */, (gpio_num_t)2  /* echo */);
    MeasurementRequest req;
    Measurement meas;

    char *buf = (char *)malloc(1024);

    while (true) {
        // Check stdin for a request
        auto size = fread(buf, 1, 3, stdin);
        if (size > 0) {
            std::cout << "Received request" << std::endl;
            std::cout << buf << std::endl;
        }
        // int len = uart_read_bytes(UART_NUM_0, (uint8_t *)buf, sizeof(buf), 100 / portTICK_PERIOD_MS);
        // std::cout << "Received request: " << len << std::endl;


        meas.distance_mm = sensor.get_distance_mm();
        json j;
        j["distance"] = meas.distance_mm;
        std::cout << j.dump() << std::endl;
        std::cout << "Distance: " << meas.distance_mm << " mm" << std::endl;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    std::cout << "Unexpected exit" << std::endl;
    // printf("Unexpected exit\n");
    fflush(stdout);
    esp_restart();
}

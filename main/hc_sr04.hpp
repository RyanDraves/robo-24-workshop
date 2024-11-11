#pragma once

#include <cinttypes>

#include "driver/gpio.h"

/* HC_SR04 Ultrasonic Sensor
 *
 * A cheap ultrasonic sensor that can be used to measure distance.
 *
 * Datasheet: https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
 * Range: 2cm to 400cm
 * Resolution: 3mm
 *
 * Reference implementation:
 * https://github.com/sparkfun/HC-SR04_UltrasonicSensor/blob/546d01c07ed2047f20b9835cb505dd3b37467bfa/Firmware/HC-SR04_UltrasonicSensorExample/HC-SR04_UltrasonicSensorExample.ino
 */
class HcSr04 {
    static constexpr uint32_t kMaxDistanceUs = 23200;

  public:
    HcSr04(gpio_num_t trigger_gpio, gpio_num_t echo_gpio);
    ~HcSr04();

    uint32_t get_distance_mm();

  private:
    gpio_num_t trigger_;
    gpio_num_t echo_;
};
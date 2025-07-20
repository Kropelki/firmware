#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#include <memory>

extern String log_buffer;

/**
 * Represents a measurement of various weather parameters
 *
 * This struct holds smart pointers to float values for different weather data.
 * Using std::unique_ptr makes sure the memory is cleaned up automatically.
 * If a pointer is null, it means that the measurement was not taken.
 */
struct Measurement {
    std::unique_ptr<float> temperature = nullptr;
    std::unique_ptr<float> humidity = nullptr;
    std::unique_ptr<float> pressure = nullptr;
    std::unique_ptr<float> dew_point = nullptr;
    std::unique_ptr<float> illumination = nullptr;
    std::unique_ptr<float> battery_voltage = nullptr;
    std::unique_ptr<float> solar_panel_voltage = nullptr;
};

/**
 * Removes any measurement that is outside sensor's valid range
 */
void remove_invalid_measurements(Measurement &measurement);

/**
 * Logs a message to both the serial output and an internal log buffer
 */
void serial_log(String message);

/**
 * Calculates the dew point temperature from temperature and humidity
 *
 * Uses the Magnus formula to calculate the dew point temperature. The dew point is the temperature
 * the air needs to be cooled to (at constant pressure) in order to produce a relative humidity of
 * 100%.
 *
 * Reference: https://en.wikipedia.org/wiki/Dew_point
 *
 * @param temperature Temperature in degrees Celsius
 * @param humidity Relative humidity as a percentage (0-100)
 * @return Dew point temperature in degrees Celsius
 */
float calculate_dew_point(float temperature, float humidity);

/**
 * Isolates all RTC-capable GPIO pins to reduce power consumption
 *
 * This function isolates all GPIO pins that can be controlled by the RTC during
 * deep sleep mode. Isolation prevents current leakage and reduces power consumption
 * when the ESP32 enters deep sleep mode. This is essential for battery-powered
 * applications to maximize battery life.
 */
void isolate_all_rtc_gpio();

/**
 * Establishes a WiFi connection using credentials from env.h
 */
void connect_to_wifi();

/**
 * Sends accumulated log messages to a remote log server
 *
 * @note Connection failures are logged but do not block execution
 */
void send_log();

/**
 * Sends weather sensor data to a remote database via HTTP GET request
 *
 * @param temperature Temperature reading in degrees Celsius
 * @param humidity Relative humidity as a percentage (0-100)
 * @param pressure Atmospheric pressure in appropriate units
 * @param dew_point Calculated dew point temperature in degrees Celsius
 * @param illumination Light level reading from light sensor
 * @param battery_voltage Current battery voltage level
 * @param solar_panel_voltage Solar panel output voltage
 */
void send_to_database(float temperature, float humidity, float pressure, float dew_point,
    float illumination, float battery_voltage, float solar_panel_voltage);

#endif // UTILS_H

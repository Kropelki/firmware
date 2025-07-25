#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

extern String log_buffer;

/**
 * Logs a message to both the serial output and an internal log buffer
 */
void serial_log(String message);

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
void send_to_database(float temperature, float humidity, float pressure, float dew_point, float illumination, float battery_voltage,
    float solar_panel_voltage);

#endif // UTILS_H

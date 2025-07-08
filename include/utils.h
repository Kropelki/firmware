#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

extern String logBuffer;

void serial_log(String message);

float calculate_dew_point(float temperature_c, float humidity);

void isolate_all_rtc_gpio();

void connect_to_wifi();

void send_log();

void send_to_database(float temperature_c, float humidity, float pressure, float dewpoint_c,
    float illumination, float battery_voltage, float solar_panel_voltage);

#endif // UTILS_H

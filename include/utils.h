#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

extern String logBuffer;

void serialLog(String message);

float calculateDewPoint(float temperature_c, float humidity);

void isolate_all_rtc_gpio();

void connectToWiFi();

void sendLog();

void sendToDatabase(float temperature_c, float humidity, float pressure, float dewpoint_c,
    float illumination, float battery_voltage, float solar_panel_voltage);

#endif // UTILS_H

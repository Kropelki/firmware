#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

extern String logBuffer;

void serialLog(String message);

float calculateDewPoint(float temperature_c, float humidity);

void isolate_all_rtc_gpio();

void connectToWiFi();

void sendLog();

#endif // UTILS_H

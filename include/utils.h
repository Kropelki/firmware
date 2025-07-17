#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

extern String log_buffer;

void serial_log(String message);

float calculate_dew_point(float temperature, float humidity);

void isolate_all_rtc_gpio();

void connect_to_wifi();

void send_log();

void send_to_database(float temperature, float humidity, float pressure, float dew_point,
    float illumination, float battery_voltage, float solar_panel_voltage);

// Power management functions
void optimize_cpu_frequency(bool high_performance);
void disable_unused_peripherals();
void enable_wifi_power_save();
void optimize_adc_power();
void prepare_for_deep_sleep();

#endif // UTILS_H

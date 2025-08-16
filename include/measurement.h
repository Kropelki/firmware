#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

#include <memory>

/**
 * Represents a measurement of various weather parameters
 *
 * This struct holds smart pointers to float values for different weather data.
 * Using std::unique_ptr makes sure the memory is cleaned up automatically.
 * If a pointer is null, it means that the measurement was not taken or is invalid.
 */
struct Measurement {
    std::unique_ptr<float> temperature_c;
    std::unique_ptr<float> temperature_f;
    std::unique_ptr<float> humidity;
    std::unique_ptr<float> pressure_hpa;
    std::unique_ptr<float> pressure_b;
    std::unique_ptr<float> dew_point_c;
    std::unique_ptr<float> dew_point_f;
    std::unique_ptr<float> illumination;
    std::unique_ptr<float> battery_voltage;
    std::unique_ptr<float> solar_panel_voltage;

    Measurement();
    void read_sensors_and_voltage(Adafruit_BMP280& bmp_sensor, Adafruit_AHTX0& aht_sensor, BH1750& light_meter,
        int solar_panel_voltage_pin, int battery_voltage_pin, float voltage_multiplier);
    void remove_invalid_measurements();
    void calculate_derived_values();
    void print_all_values() const;
    bool has_sensor_data() const;
};

#endif // MEASUREMENT_H

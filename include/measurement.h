#ifndef MEASUREMENT_H
#define MEASUREMENT_H

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
    void calculateDerivedValues();
    void remove_invalid_measurements();
    bool hasSensorData() const;
};

#endif // MEASUREMENT_H

#include "measurement.h"
#include <math.h>

static float calculate_dew_point(float temperature, float humidity);

Measurement::Measurement()
    : temperature_c(nullptr)
    , temperature_f(nullptr)
    , humidity(nullptr)
    , pressure_hpa(nullptr)
    , pressure_b(nullptr)
    , dew_point_c(nullptr)
    , dew_point_f(nullptr)
    , illumination(nullptr)
    , battery_voltage(nullptr)
    , solar_panel_voltage(nullptr)
{
}

void Measurement::calculateDerivedValues()
{
    if (temperature_c) {
        temperature_f = std::make_unique<float>(*temperature_c * 9.0f / 5.0f + 32.0f);
        if (humidity) {
            dew_point_c = std::make_unique<float>(calculate_dew_point(*temperature_c, *humidity));
            dew_point_f = std::make_unique<float>(*dew_point_c * 9.0f / 5.0f + 32.0f);
        }
    }
    if (pressure_hpa) {
        pressure_b = std::make_unique<float>(*pressure_hpa * 0.02953f);
    }
}

void Measurement::remove_invalid_measurements()
{
    // TODO: link to data sheet for each sensor about valid ranges
    if (temperature_c)
        if (*temperature_c < -40 || *temperature_c > 85)
            temperature_c = nullptr;
    if (humidity)
        if (*humidity < 0 || *humidity > 100)
            humidity = nullptr;
    if (pressure_hpa)
        if (*pressure_hpa < 300 || *pressure_hpa > 1100)
            pressure_hpa = nullptr;
    if (illumination)
        if (*illumination < 0 || *illumination > 100000)
            illumination = nullptr;
}

bool Measurement::hasSensorData() const
{
    // we generally don't want to send data if we don't have at least one sensor reading
    return temperature_c != nullptr || humidity != nullptr || pressure_hpa != nullptr || illumination != nullptr;
}

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
static float calculate_dew_point(float temperature, float humidity)
{
    const float b = 17.62;
    const float c = 243.12;
    float alpha = ((b * temperature) / (c + temperature)) + log(humidity / 100.0);
    return (c * alpha) / (b - alpha);
}

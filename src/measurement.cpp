#include <math.h>

#include "measurement.h"
#include "utils.h"

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
    , battery_voltage_a0(nullptr)
    , solar_panel_voltage_a1(nullptr)
    , uv_voltage_a2(nullptr)
    , uv_index(nullptr)
{
}

void Measurement::read_sensors_and_voltage(
    Adafruit_BMP280& bmp_sensor, Adafruit_AHTX0& aht_sensor, BH1750& light_meter, Adafruit_ADS1115& ads_sensor)
{
    if (bmp_sensor.begin(0x77))
        pressure_hpa = std::make_unique<float>(bmp_sensor.readPressure() / 100.0); // Pa to hPa conversion
    else
        serial_log("Could not find BMP280!");

    if (aht_sensor.begin()) {
        sensors_event_t hum, temp;
        aht_sensor.getEvent(&hum, &temp);
        temperature_c = std::make_unique<float>(temp.temperature);
        humidity = std::make_unique<float>(hum.relative_humidity);
    } else
        serial_log("Could not find AHT20!");

    // TODOFIX: the sensor does not work
    // if (light_meter.begin())
    //     illumination = std::make_unique<float>(light_meter.readLightLevel());
    // else
    //     serial_log("Could not find BH1750!");

    if (ads_sensor.begin()) {
        // GAIN_ONE: +/-4.096V range (for battery and solar panel voltage)
        ads_sensor.setGain(GAIN_ONE);

        int16_t adc0 = ads_sensor.readADC_SingleEnded(0);
        int16_t adc1 = ads_sensor.readADC_SingleEnded(1);
        float voltage0 = ads_sensor.computeVolts(adc0);
        float voltage1 = ads_sensor.computeVolts(adc1);

        // GAIN_FOUR: +/-1.024V range (for UV sensor voltage)
        ads_sensor.setGain(GAIN_FOUR);

        int16_t adc2 = ads_sensor.readADC_SingleEnded(2);
        float voltage2 = ads_sensor.computeVolts(adc2);

        // When there's no signal or very weak signal,
        // ADCs can return small negative values like -0.0, -0.001, etc.
        float corrected_voltage0 = max(0.0f, voltage0);
        float corrected_voltage1 = max(0.0f, voltage1);
        float corrected_voltage2 = max(0.0f, voltage2);

        battery_voltage_a0 = std::make_unique<float>((corrected_voltage0 * 1.33) + 0.03); // +0.03V calibration offset
        solar_panel_voltage_a1 = std::make_unique<float>(corrected_voltage1 * 2.43);
        uv_voltage_a2 = std::make_unique<float>(corrected_voltage2);
    } else {
        serial_log("Could not find ADS1115!");
    }
}

void Measurement::remove_invalid_measurements()
{
    /*
        BMP280 (pressure):
            https://www.alldatasheet.com/datasheet-pdf/view/1132069/BOSCH/BMP280.html
        AHT20 (temperature and humidity):
            https://static.maritex.eu/file/display/RNvX5GenZti93oVcmXPk9n_PKbFzX2F0/AHT20.pdf
        BH1750 (illumination):
            https://www.handsontec.com/dataspecs/sensor/BH1750%20Light%20Sensor.pdf
    */
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
        if (*illumination < 0 || *illumination > 65535)
            illumination = nullptr;
}

void Measurement::calculate_derived_values()
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
    if (uv_voltage_a2) {
        float calculated_uv_index = (*uv_voltage_a2) * 10.0;
        calculated_uv_index = max(0.0f, min(calculated_uv_index, 12.0f));
        uv_index = std::make_unique<int>(round(calculated_uv_index));
    }
}

bool Measurement::has_sensor_data() const
{
    // we generally don't want to send data if we don't have at least one sensor reading
    return temperature_c != nullptr || humidity != nullptr || pressure_hpa != nullptr || illumination != nullptr;
}

void Measurement::print_all_values() const
{
    if (temperature_c)
        serial_log("Temperature: " + String(*temperature_c, 2) + " °C (" + String(*temperature_f, 2) + " °F)");
    if (humidity)
        serial_log("Humidity: " + String(*humidity, 1) + " %");
    if (pressure_hpa)
        serial_log("Pressure: " + String(*pressure_hpa, 2) + " hPa (" + String(*pressure_b, 2) + " inHg)");
    if (dew_point_c)
        serial_log("Dew Point: " + String(*dew_point_c, 2) + " °C (" + String(*dew_point_f, 2) + " °F)");
    if (illumination)
        serial_log("Illumination: " + String(*illumination, 1) + " lx");
    if (battery_voltage_a0)
        serial_log("Battery voltage: " + String(*battery_voltage_a0, 2) + " V");
    if (solar_panel_voltage_a1)
        serial_log("Solar panel voltage: " + String(*solar_panel_voltage_a1, 2) + " V");
    if (uv_voltage_a2)
        serial_log("UV voltage: " + String(*uv_voltage_a2, 2) + " V");
    if (uv_index)
        serial_log("UV Index: " + String(*uv_index));
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

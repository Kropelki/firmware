#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <WiFi.h>
#include <Wire.h>
#include <memory>

#include "env.h"
#include "influxdb.h"
#include "utils.h"
#include "wunderground.h"

const float voltage_multiplier = 3.2;

#define BMP280_AHT20_PIN 12
#define BH1750_PIN 14
#define SOLAR_PANEL_VOLTAGE_PIN 35
#define BATTERY_VOLTAGE_PIN 34

Adafruit_BMP280 bmp_sensor; // BMP280: measures pressure
Adafruit_AHTX0 aht_sensor; // AHT20: measures temperature and humidity
BH1750 light_meter; // BH1750: measures illumination

void setup()
{
    unsigned long startTime = millis();
    btStop();

    pinMode(BMP280_AHT20_PIN, OUTPUT);
    pinMode(BH1750_PIN, OUTPUT);
    digitalWrite(BMP280_AHT20_PIN, HIGH);
    digitalWrite(BH1750_PIN, HIGH);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    Serial.begin(115200);
    delay(1000);

    Wire.begin(21, 22); // SDA, SCL
    connect_to_wifi();

    Measurement measurement;

    if (bmp_sensor.begin(0x77)) {
        measurement.pressure_hpa = std::make_unique<float>(bmp_sensor.readPressure() / 100.0); // Pa to hPa conversion
    } else {
        serial_log("Could not find BMP280!");
    }

    if (aht_sensor.begin()) {
        sensors_event_t hum, temp;
        aht_sensor.getEvent(&hum, &temp);
        measurement.temperature_c = std::make_unique<float>(temp.temperature);
        measurement.humidity = std::make_unique<float>(hum.relative_humidity);
    } else {
        serial_log("Could not find AHT20!");
    }

    if (light_meter.begin()) {
        measurement.illumination = std::make_unique<float>(light_meter.readLightLevel());
    } else {
        serial_log("Could not find BH1750!");
    }

    int raw = analogRead(SOLAR_PANEL_VOLTAGE_PIN);
    measurement.solar_panel_voltage = std::make_unique<float>((raw / 4095.0) * 3.3 * voltage_multiplier);
    raw = analogRead(BATTERY_VOLTAGE_PIN);
    measurement.battery_voltage = std::make_unique<float>((raw / 4095.0) * 3.3 * voltage_multiplier);

    measurement.remove_invalid_measurements();
    measurement.calculateDerivedValues();

    if (measurement.temperature_c)
        serial_log(
            "Temperature: " + String(*measurement.temperature_c, 2) + " °C (" + String(*measurement.temperature_f, 2) + " °F)");
    if (measurement.humidity)
        serial_log("Humidity: " + String(*measurement.humidity, 1) + " %");
    if (measurement.pressure_hpa)
        serial_log("Pressure: " + String(*measurement.pressure_hpa, 2) + " hPa (" + String(*measurement.pressure_b, 2) + " inHg)");
    if (measurement.dew_point_c)
        serial_log("Dew Point: " + String(*measurement.dew_point_c, 2) + " °C (" + String(*measurement.dew_point_f, 2) + " °F)");
    if (measurement.illumination)
        serial_log("Illumination: " + String(*measurement.illumination, 1) + " lx");
    if (measurement.battery_voltage)
        serial_log("Battery voltage: " + String(*measurement.battery_voltage, 2) + " V");
    if (measurement.solar_panel_voltage)
        serial_log("Solar panel voltage: " + String(*measurement.solar_panel_voltage, 2) + " V");

    unsigned long activeTime = (millis() - startTime) / 1000;

    if (SEND_TO_EXTERNAL_SERVICES) {
        send_to_wunderground(measurement);
        send_to_influx_db(measurement);
    } else {
        serial_log("External services sending is disabled.");
    }

    send_log();

    isolate_all_rtc_gpio();
    WiFi.mode(WIFI_OFF);

    unsigned long sleepTime = (CYCLE_TIME_SEC - activeTime) * 1000000;
    serial_log("Entering deep sleep for " + String(sleepTime / 1000000) + " seconds...");

    esp_sleep_enable_timer_wakeup(sleepTime);
    esp_deep_sleep_start();
}

void loop() { }

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
        measurement.pressure = std::make_unique<float>(bmp_sensor.readPressure() / 100.0); // Pa to hPa conversion
    } else {
        serial_log("Could not find BMP280!");
    }

    if (aht_sensor.begin()) {
        sensors_event_t hum, temp;
        aht_sensor.getEvent(&hum, &temp);
        measurement.temperature = std::make_unique<float>(temp.temperature);
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
    float solar_panel_voltage = (raw / 4095.0) * 3.3 * voltage_multiplier;
    raw = analogRead(BATTERY_VOLTAGE_PIN);
    float battery_voltage = (raw / 4095.0) * 3.3 * voltage_multiplier;

    float temperature_f = *measurement.temperature * 9.0 / 5.0 + 32.0;
    float baromin = *measurement.pressure * 0.02953;
    float dew_point_c = calculate_dew_point(*measurement.temperature, *measurement.humidity);
    float dew_point_f = dew_point_c * 9.0 / 5.0 + 32.0;

    serial_log("Temperature: " + String(*measurement.temperature, 2) + " °C (" + String(temperature_f, 2) + " °F)");
    serial_log("Humidity: " + String(*measurement.humidity, 1) + " %");
    serial_log("Pressure: " + String(*measurement.pressure, 2) + " hPa");
    serial_log("Baromin: " + String(baromin, 2) + " inHg");
    serial_log("Dew Point: " + String(dew_point_c, 2) + " °C (" + String(dew_point_f, 2) + " °F)");
    serial_log("Illumination: " + String(*measurement.illumination, 1) + " lx");
    serial_log("Battery voltage: " + String(battery_voltage, 2) + " V");
    serial_log("Solar panel voltage: " + String(solar_panel_voltage, 2) + " V");

    unsigned long activeTime = (millis() - startTime) / 1000;

    if (*measurement.temperature >= -40 && *measurement.temperature <= 85 && *measurement.humidity >= 0 && *measurement.humidity <= 100 && *measurement.pressure >= 300 && *measurement.pressure <= 1100) {
        if (SEND_TO_EXTERNAL_SERVICES) {
            send_to_wunderground(temperature_f, *measurement.humidity, baromin, dew_point_f);
            send_to_influx_db(*measurement.temperature, *measurement.humidity, *measurement.pressure, dew_point_c, *measurement.illumination, battery_voltage, solar_panel_voltage);
        } else {
            serial_log("External services sending is disabled.");
        }

    } else {
        serial_log("Can not send data to WeatherUnderground and InfluxDB");
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

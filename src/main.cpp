#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <math.h>

#include "env.h"
#include "influxdb.h"
#include "utils.h"
#include "wunderground.h"

const float voltage_multiplier = 3.2;

#define BMP280_AHT20_PIN 12
#define BH1750_PIN 14
#define SOLAR_PANEL_VOLTAGE_PIN 35
#define BATTERY_VOLTAGE_PIN 34

// Sensor instances
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
BH1750 lightMeter;

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

    float illumination = 0;
    float temperature_c = -1000;
    float humidity = -1000;
    float pressure = -1000;

    if (bmp.begin(0x77)) {
        pressure = bmp.readPressure() / 100.0;
    } else {
        serial_log("Could not find BMP280!");
    }

    if (aht.begin()) {
        sensors_event_t hum, temp;
        aht.getEvent(&hum, &temp);
        temperature_c = temp.temperature;
        humidity = hum.relative_humidity;
    } else {
        serial_log("Could not find AHT20!");
    }

    if (lightMeter.begin()) {
        float illumination = lightMeter.readLightLevel();
    } else {
        serial_log("Could not find BH1750!");
    }

    int raw = analogRead(SOLAR_PANEL_VOLTAGE_PIN);
    float solar_panel_voltage = (raw / 4095.0) * 3.3 * voltage_multiplier;
    raw = analogRead(BATTERY_VOLTAGE_PIN);
    float battery_voltage = (raw / 4095.0) * 3.3 * voltage_multiplier;

    float temperature_f = temperature_c * 9.0 / 5.0 + 32.0;
    float baromin = pressure * 0.02953;
    float dewpoint_c = calculate_dew_point(temperature_c, humidity);
    float dewpoint_f = dewpoint_c * 9.0 / 5.0 + 32.0;

    serial_log(
        "Temperature: " + String(temperature_c, 2) + " °C (" + String(temperature_f, 2) + " °F)");
    serial_log("Humidity: " + String(humidity, 1) + " %");
    serial_log("Pressure: " + String(pressure, 2) + " hPa");
    serial_log("Baromin: " + String(baromin, 2) + " inHg");
    serial_log("Dew Point: " + String(dewpoint_c, 2) + " °C (" + String(dewpoint_f, 2) + " °F)");
    serial_log("Illumination: " + String(illumination, 1) + " lx");
    serial_log("Battery voltage: " + String(battery_voltage, 2) + " V");
    serial_log("Solar panel voltage: " + String(solar_panel_voltage, 2) + " V");

    unsigned long activeTime = (millis() - startTime) / 1000;
    // send_to_database(temperature_c, humidity, pressure, dewpoint_c,
    // illumination, battery_voltage, solar_panel_voltage);
    send_to_influx_db(temperature_c, humidity, pressure, dewpoint_c, illumination, battery_voltage,
        solar_panel_voltage);

    if (temperature_c != -1000 || humidity != -1000 || pressure != -1000) {
        if (SEND_TO_WEATHER_UNDERGROUND) {
            send_to_wunderground(temperature_f, humidity, baromin, dewpoint_f);
        } else {
            serial_log("WeatherUnderground sending is disabled.");
        }
    } else {
        serial_log("Can not send data to WeatherUnderground");
    }

    send_log();

    isolate_all_rtc_gpio();
    WiFi.mode(WIFI_OFF);
    unsigned long sleepTime = (CYCLE_TIME_SEC - activeTime) * 1000000;
    serial_log("Entering deep sleep for " + String(sleepTime / 1000000) + " seconds...");
    esp_sleep_enable_timer_wakeup(sleepTime);
    esp_deep_sleep_start();
}

void loop()
{
    // Empty, everything is done in setup()
}

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

const float voltage_multiplier = 3.2;

#define BMP280_AHT20_PIN 12
#define BH1750_PIN 14
#define SOLAR_PANEL_VOLTAGE_PIN 35
#define BATTERY_VOLTAGE_PIN 34

// Sensor instances
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
BH1750 lightMeter;

void sendToDatabase(float temperature_c, float humidity, float pressure, float dewpoint_c,
    float illumination, float battery_voltage, float solar_panel_voltage)
{
    HTTPClient http;
    String url = String(TEST_SERVER_HOST) + ":" + String(TEST_SERVER_PORT) + "/api/weather"
        + "?temperature=" + String(temperature_c, 2) + "&dew_point=" + String(dewpoint_c, 2)
        + "&humidity=" + String(humidity, 1) + "&illumination=" + String(illumination, 1)
        + "&pressure=" + String(pressure, 2) + "&battery_voltage=" + String(battery_voltage, 2)
        + "&solar_panel_voltage=" + String(solar_panel_voltage, 2);

    serialLog("Sending to: " + url);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
        serialLog("Response: " + http.getString());
    } else {
        serialLog("Error on sending request");
    }
    http.end();
}

void sendToWeatherUnderground(float temperature_f, int humidity, float baromin, float dewpoint_f)
{
    if (WiFi.status() == WL_CONNECTED) {
        String url = "http://weatherstation.wunderground.com/weatherstation/"
                     "updateweatherstation.php";
        url += "?ID=" + String(WEATHER_UNDERGROUND_STATION_ID);
        url += "&PASSWORD=" + String(WEATHER_UNDERGROUND_API_KEY);
        url += "&dateutc=now";
        url += "&tempf=" + String(temperature_f, 2);
        url += "&dewptf=" + String(dewpoint_f, 2);
        url += "&humidity=" + String(humidity);
        url += "&baromin=" + String(baromin, 2);
        url += "&action=updateraw";

        Serial.println("Sending data: " + url);

        HTTPClient http;
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("Response: " + payload);
        } else {
            Serial.println("Sending error: " + http.errorToString(httpCode));
        }

        http.end();
    } else {
        serialLog("WiFi not connected");
    }
}

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
    connectToWiFi();

    float illumination = 0;
    float temperature_c = -1000;
    float humidity = -1000;
    float pressure = -1000;

    if (bmp.begin(0x77)) {
        pressure = bmp.readPressure() / 100.0;
    } else {
        serialLog("Could not find BMP280!");
    }

    if (aht.begin()) {
        sensors_event_t hum, temp;
        aht.getEvent(&hum, &temp);
        temperature_c = temp.temperature;
        humidity = hum.relative_humidity;
    } else {
        serialLog("Could not find AHT20!");
    }

    if (lightMeter.begin()) {
        float illumination = lightMeter.readLightLevel();
    } else {
        serialLog("Could not find BH1750!");
    }

    int raw = analogRead(SOLAR_PANEL_VOLTAGE_PIN);
    float solar_panel_voltage = (raw / 4095.0) * 3.3 * voltage_multiplier;
    raw = analogRead(BATTERY_VOLTAGE_PIN);
    float battery_voltage = (raw / 4095.0) * 3.3 * voltage_multiplier;

    float temperature_f = temperature_c * 9.0 / 5.0 + 32.0;
    float baromin = pressure * 0.02953;
    float dewpoint_c = calculateDewPoint(temperature_c, humidity);
    float dewpoint_f = dewpoint_c * 9.0 / 5.0 + 32.0;

    serialLog(
        "Temperature: " + String(temperature_c, 2) + " °C (" + String(temperature_f, 2) + " °F)");
    serialLog("Humidity: " + String(humidity, 1) + " %");
    serialLog("Pressure: " + String(pressure, 2) + " hPa");
    serialLog("Baromin: " + String(baromin, 2) + " inHg");
    serialLog("Dew Point: " + String(dewpoint_c, 2) + " °C (" + String(dewpoint_f, 2) + " °F)");
    serialLog("Illumination: " + String(illumination, 1) + " lx");
    serialLog("Battery voltage: " + String(battery_voltage, 2) + " V");
    serialLog("Solar panel voltage: " + String(solar_panel_voltage, 2) + " V");

    unsigned long activeTime = (millis() - startTime) / 1000;
    // sendToDatabase(temperature_c, humidity, pressure, dewpoint_c,
    // illumination, battery_voltage, solar_panel_voltage);
    sendToInfluxDB(temperature_c, humidity, pressure, dewpoint_c, illumination, battery_voltage,
        solar_panel_voltage);

    if (temperature_c != -1000 || humidity != -1000 || pressure != -1000) {
        sendToWeatherUnderground(temperature_f, humidity, baromin, dewpoint_f);
    } else {
        serialLog("Can not send data to WeatherUnderground");
    }

    sendLog();

    isolate_all_rtc_gpio();
    WiFi.mode(WIFI_OFF);
    unsigned long sleepTime = (CYCLE_TIME_SEC - activeTime) * 1000000;
    serialLog("Entering deep sleep for " + String(sleepTime / 1000000) + " seconds...");
    esp_sleep_enable_timer_wakeup(sleepTime);
    esp_deep_sleep_start();
}

void loop()
{
    // Empty, everything is done in setup()
}

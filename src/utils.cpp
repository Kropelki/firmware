#include "utils.h"
#include "env.h"

#include "driver/rtc_io.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

String log_buffer = "";

Measurement::Measurement()
    : temperature_c(nullptr), temperature_f(nullptr), humidity(nullptr), pressure_hpa(nullptr),
      pressure_b(nullptr), dew_point_c(nullptr), dew_point_f(nullptr), illumination(nullptr), battery_voltage(nullptr),
      solar_panel_voltage(nullptr) {}

void Measurement::calculateDerivedValues() {
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

void serial_log(String message)
{
    log_buffer += message + "\n";
    Serial.println(message);
}

float calculate_dew_point(float temperature, float humidity)
{
    const float b = 17.62;
    const float c = 243.12;
    float alpha = ((b * temperature) / (c + temperature)) + log(humidity / 100.0);
    return (c * alpha) / (b - alpha);
}

void isolate_all_rtc_gpio()
{
    const gpio_num_t rtc_gpio_list[] = { GPIO_NUM_0, GPIO_NUM_2, GPIO_NUM_4, GPIO_NUM_12,
        GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_32,
        GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39 };

    for (int i = 0; i < sizeof(rtc_gpio_list) / sizeof(rtc_gpio_list[0]); i++) {
        rtc_gpio_isolate(rtc_gpio_list[i]);
    }
}

void connect_to_wifi()
{
    serial_log("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            serial_log("\nWiFi connected!");
            serial_log(WiFi.localIP().toString());
            return;
        }
        if (WiFi.status() == WL_NO_SSID_AVAIL) {
            Serial.println("SSID not found!");
            Serial.println("Entering deep sleep for 5 minutes.");
            esp_sleep_enable_timer_wakeup(300000000);
            esp_deep_sleep_start();
        }
        delay(500);
        serial_log(".");
    }

    Serial.println("Response: " + String(WiFi.status()));
    ESP.restart();
}

void send_log()
{
    WiFiClient client;
    if (client.connect(LOG_SERVER_HOST, LOG_SERVER_PORT)) {
        int contentLength = log_buffer.length();
        client.print(String("POST ") + LOG_SERVER_PATH + " HTTP/1.1\r\n" + "Host: "
            + LOG_SERVER_HOST + "\r\n" + "Content-Type: text/plain\r\n" + "Content-Length: "
            + String(contentLength) + "\r\n" + "Connection: close\r\n\r\n" + log_buffer);

        delay(10);
        client.stop();

        serial_log("Log sent synchronously (no response expected).");
    } else {
        serial_log("Failed to connect to the log server.");
    }
}

void send_to_database(float temperature, float humidity, float pressure, float dew_point,
    float illumination, float battery_voltage, float solar_panel_voltage)
{
    HTTPClient http;
    String url = String(TEST_SERVER_HOST) + ":" + String(TEST_SERVER_PORT) + "/api/weather"
        + "?temperature=" + String(temperature, 2) + "&dew_point=" + String(dew_point, 2)
        + "&humidity=" + String(humidity, 1) + "&illumination=" + String(illumination, 1)
        + "&pressure=" + String(pressure, 2) + "&battery_voltage=" + String(battery_voltage, 2)
        + "&solar_panel_voltage=" + String(solar_panel_voltage, 2);

    serial_log("Sending to: " + url);
    http.begin(url);
    int http_code = http.GET();
    if (http_code > 0) {
        serial_log("Response: " + http.getString());
    } else {
        serial_log("Error on sending request");
    }
    http.end();
}

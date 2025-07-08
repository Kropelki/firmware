#include "utils.h"
#include "env.h"

#include "driver/rtc_io.h"

#include <HTTPClient.h>

String log_buffer = "";

/**
 * Logs a message to both the serial output and an internal log buffer
 */
void serial_log(String message)
{
    log_buffer += message + "\n";
    Serial.println(message);
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
float calculate_dew_point(float temperature, float humidity)
{
    const float b = 17.62;
    const float c = 243.12;
    float alpha = ((b * temperature) / (c + temperature)) + log(humidity / 100.0);
    return (c * alpha) / (b - alpha);
}

/**
 * Isolates all RTC-capable GPIO pins to reduce power consumption
 *
 * This function isolates all GPIO pins that can be controlled by the RTC during
 * deep sleep mode. Isolation prevents current leakage and reduces power consumption
 * when the ESP32 enters deep sleep mode. This is essential for battery-powered
 * applications to maximize battery life.
 */
void isolate_all_rtc_gpio()
{
    const gpio_num_t rtc_gpio_list[] = { GPIO_NUM_0, GPIO_NUM_2, GPIO_NUM_4, GPIO_NUM_12,
        GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_32,
        GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39 };

    for (int i = 0; i < sizeof(rtc_gpio_list) / sizeof(rtc_gpio_list[0]); i++) {
        rtc_gpio_isolate(rtc_gpio_list[i]);
    }
}

/**
 * Establishes a WiFi connection using credentials from env.h
 *
 * @note This function will block indefinitely if WiFi connection fails
 * TODO(FIX): this surely shouldn't be implemented this way
 * TODO(FIX): we should FIX it ASAP as it already caused issues
 */
void connect_to_wifi()
{
    serial_log("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        serial_log(".");
    }
    serial_log("\nWiFi connected!");
    serial_log(WiFi.localIP().toString());
}

/**
 * Sends accumulated log messages to a remote log server
 *
 * @note Connection failures are logged but do not block execution
 */
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

/**
 * Sends weather sensor data to a remote database via HTTP GET request
 *
 * @param temperature Temperature reading in degrees Celsius
 * @param humidity Relative humidity as a percentage (0-100)
 * @param pressure Atmospheric pressure in appropriate units
 * @param dew_point Calculated dew point temperature in degrees Celsius
 * @param illumination Light level reading from light sensor
 * @param battery_voltage Current battery voltage level
 * @param solar_panel_voltage Solar panel output voltage
 */
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

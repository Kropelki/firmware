#include "env.h"
#include "utils.h"

#include "driver/rtc_io.h"

#include <HTTPClient.h>

String logBuffer = "";

void serialLog(String message)
{
    logBuffer += message + "\n";
    Serial.println(message);
}

float calculateDewPoint(float temperature_c, float humidity)
{
    const float a = 17.62;
    const float b = 243.12;
    float alpha = ((a * temperature_c) / (b + temperature_c)) + log(humidity / 100.0);
    return (b * alpha) / (a - alpha);
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

void connectToWiFi()
{
    serialLog("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        serialLog(".");
    }
    serialLog("\nWiFi connected!");
    serialLog(WiFi.localIP().toString());
}

void sendLog()
{
    WiFiClient client;
    if (client.connect(LOG_SERVER_HOST, LOG_SERVER_PORT)) {
        int contentLength = logBuffer.length();
        client.print(String("POST ") + LOG_SERVER_PATH + " HTTP/1.1\r\n" + "Host: "
            + LOG_SERVER_HOST + "\r\n" + "Content-Type: text/plain\r\n" + "Content-Length: "
            + String(contentLength) + "\r\n" + "Connection: close\r\n\r\n" + logBuffer);

        delay(10);
        client.stop();

        serialLog("Log sent synchronously (no response expected).");
    } else {
        serialLog("Failed to connect to the log server.");
    }
}

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

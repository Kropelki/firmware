#include "env.h"
#include "utils.h"

#include <HTTPClient.h>
#include <WiFi.h>

void send_to_wunderground(const Measurement& measurement)
{
    if (WiFi.status() == WL_CONNECTED) {
        String url = "http://weatherstation.wunderground.com/weatherstation/"
                     "updateweatherstation.php";
        url += "?ID=" + String(WEATHER_UNDERGROUND_STATION_ID);
        url += "&PASSWORD=" + String(WEATHER_UNDERGROUND_API_KEY);
        url += "&dateutc=now";
        if (measurement.temperature_f)
            url += "&tempf=" + String(*measurement.temperature_f, 2);
        if (measurement.dew_point_f)
            url += "&dewptf=" + String(*measurement.dew_point_f, 2);
        if (measurement.humidity)
            url += "&humidity=" + String(*measurement.humidity);
        if (measurement.pressure_b)
            url += "&baromin=" + String(*measurement.pressure_b, 2);
        url += "&action=updateraw";

        Serial.println("Sending data: " + url);

        HTTPClient http;
        http.begin(url);
        int http_code = http.GET();

        if (http_code > 0) {
            String payload = http.getString();
            Serial.println("Response: " + payload);
        } else {
            Serial.println("Sending error: " + http.errorToString(http_code));
        }

        delay(10);
        http.end();
    } else {
        serial_log("WiFi not connected");
    }
}

#include "env.h"
#include "utils.h"

#include <HTTPClient.h>
#include <WiFi.h>

void send_to_wunderground(float temperature, int humidity, float baromin, float dew_point)
{
    if (WiFi.status() == WL_CONNECTED) {
        String url = "http://weatherstation.wunderground.com/weatherstation/"
                     "updateweatherstation.php";
        url += "?ID=" + String(WEATHER_UNDERGROUND_STATION_ID);
        url += "&PASSWORD=" + String(WEATHER_UNDERGROUND_API_KEY);
        url += "&dateutc=now";
        url += "&tempf=" + String(temperature, 2);
        url += "&dewptf=" + String(dew_point, 2);
        url += "&humidity=" + String(humidity);
        url += "&baromin=" + String(baromin, 2);
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

        http.end();
    } else {
        serial_log("WiFi not connected");
    }
}

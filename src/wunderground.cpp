#include "env.h"
#include "utils.h"

#include <HTTPClient.h>

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

#include "env.h"
#include "utils.h"

#include <HTTPClient.h>

/**
 * Sends weather data to Weather Underground station
 *
 * This function transmits current weather measurements to the Weather Underground
 * service via HTTP GET request. The data is sent in Fahrenheit units for temperature
 * and dewpoint, with humidity as percentage and barometric pressure in inches.
 *
 * @param temperature Current temperature in Fahrenheit
 * @param humidity Relative humidity as percentage (0-100)
 * @param baromin Barometric pressure in inches of mercury
 * @param dewpoint Dew point temperature in Fahrenheit
 */
void send_to_wunderground(float temperature, int humidity, float baromin, float dewpoint)
{
    if (WiFi.status() == WL_CONNECTED) {
        String url = "http://weatherstation.wunderground.com/weatherstation/"
                     "updateweatherstation.php";
        url += "?ID=" + String(WEATHER_UNDERGROUND_STATION_ID);
        url += "&PASSWORD=" + String(WEATHER_UNDERGROUND_API_KEY);
        url += "&dateutc=now";
        url += "&tempf=" + String(temperature, 2);
        url += "&dewptf=" + String(dewpoint, 2);
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

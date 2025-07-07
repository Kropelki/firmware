#ifndef INFLUXDB_H
#define INFLUXDB_H

#include "env.h"
#include "utils.h"

#include <HTTPClient.h>

void sendToInfluxDB(float temperature_c, float humidity, float pressure, float dewpoint_c,
    float illumination, float battery_voltage, float solar_panel_voltage)
{
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        const char* InfluxDB_url = "https://eu-central-1-1.aws.cloud2.influxdata.com/api/v2/"
                                   "write?bucket=db.v0&precision=ns";

        serialLog("Sending data to InfluxDB...");

        http.begin(InfluxDB_url);
        http.setTimeout(10000);

        http.addHeader("Authorization", String("Token ") + INFLUXDB_API_TOKEN);
        http.addHeader("Content-Type", "text/plain; charset=utf-8");
        http.addHeader("Accept", "application/json");

        // String payload = "weather
        // temperature=200.37,humidity=40.2,pressure=1012,illumination=56.3,dew_point=30";
        String payload = String("weather ") + "temperature=" + String(temperature_c, 2) + ","
            + "humidity=" + String(humidity, 1) + "," + "pressure=" + String(pressure, 2) + ","
            + "illumination=" + String(illumination, 1) + "," + "dew_point=" + String(dewpoint_c, 1)
            + "," + "battery_voltage=" + String(battery_voltage, 2) + ","
            + "solar_panel_voltage=" + String(solar_panel_voltage, 2);

        int httpResponseCode = http.POST(payload);
        serialLog(payload);

        if (httpResponseCode > 0) {
            // String response = http.getString();
            serialLog("HTTP Response Code: ");
            serialLog(String(httpResponseCode));
            // serialLog("Response: ");
            // serialLog(response);
        } else {
            serialLog("Error in HTTP request: ");
            serialLog(String(httpResponseCode));
            // serialLog(http.getString());
        }

        http.end();
    } else {
        serialLog("WiFi not connected");
    }
}

#endif // INFLUXDB_H

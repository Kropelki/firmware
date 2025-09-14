#include "influxdb.h"
#include "env.h"
#include "measurement.h"
#include "utils.h"

#include <HTTPClient.h>
#include <WiFi.h>

void send_to_influx_db(const Measurement& measurement)
{
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String request_url = String(INFLUXDB_HOSTNAME) + "/api/v2/write?" + "bucket=" + String(INFLUXDB_BUCKET) + "&precision=s";

        serial_log("Sending data to InfluxDB...");

        http.begin(request_url);
        http.setTimeout(10000); // 10s

        http.addHeader("Authorization", "Token " + String(INFLUXDB_API_TOKEN));
        http.addHeader("Content-Type", "text/plain; charset=utf-8");
        http.addHeader("Accept", "application/json");

        // Format: "weather temperature=XX.XX,humidity=XX.X,pressure=XX.XX,..."
        String payload = String("weather ");
        if (measurement.temperature_c)
            payload += "temperature=" + String(*measurement.temperature_c, 2) + ",";
        if (measurement.dew_point_c)
            payload += "dew_point=" + String(*measurement.dew_point_c, 2) + ",";
        if (measurement.humidity)
            payload += "humidity=" + String(*measurement.humidity, 1) + ",";
        if (measurement.pressure_hpa)
            payload += "pressure=" + String(*measurement.pressure_hpa, 2) + ",";
        if (measurement.illumination)
            payload += "illumination=" + String(*measurement.illumination, 1) + ",";
        if (measurement.battery_voltage_a0)
            payload += "battery_voltage=" + String(*measurement.battery_voltage_a0, 2) + ",";
        if (measurement.solar_panel_voltage_a1)
            payload += "solar_panel_voltage=" + String(*measurement.solar_panel_voltage_a1, 2) + ",";
        if (measurement.uv_voltage_a2)
            payload += "uv_voltage=" + String(*measurement.uv_voltage_a2, 2);

        if (payload.endsWith(","))
            payload.remove(payload.length() - 1);

        int response_code = http.POST(payload);
        serial_log(payload);

        if (response_code > 0) {
            serial_log("HTTP Response Code: ");
            serial_log(String(response_code));
        } else {
            serial_log("Error in HTTP request: ");
            serial_log(String(response_code));
        }

        delay(10);
        http.end();
    } else {
        serial_log("WiFi not connected");
    }
}

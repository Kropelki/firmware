#include "env.h"
#include "utils.h"

#include <HTTPClient.h>

/**
 * Sends weather sensor data to InfluxDB using HTTP POST request
 *
 * This function constructs an InfluxDB line protocol payload with sensor readings
 * and transmits it to the configured InfluxDB instance via HTTP API v2.
 *
 * Link to InfluxDB API documentation:
 * https://docs.influxdata.com/influxdb3/cloud-serverless/get-started/write/?t=v2+API#write-line-protocol-to-influxdb
 *
 * @param temperature Temperature in Celsius
 * @param humidity Relative humidity percentage
 * @param pressure Atmospheric pressure
 * @param dewpoint Dew point in Celsius
 * @param illumination Light intensity measurement
 * @param battery_voltage Battery voltage level
 * @param solar_panel_voltage Solar panel voltage output
 *
 * @note Requires active WiFi connection. Function will log error if WiFi disconnected.
 * @note Uses InfluxDB API v2 with token-based authentication.
 */
void send_to_influx_db(float temperature, float humidity, float pressure, float dewpoint,
    float illumination, float battery_voltage, float solar_panel_voltage)
{
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String request_url = String(INFLUXDB_HOSTNAME) + "/api/v2/write?"
            + "bucket=" + String(INFLUXDB_BUCKET) + "&precision=ns";

        serial_log("Sending data to InfluxDB...");

        http.begin(request_url);
        http.setTimeout(10000); // 10s

        http.addHeader("Authorization", "Token " + String(INFLUXDB_API_TOKEN));
        http.addHeader("Content-Type", "text/plain; charset=utf-8");
        http.addHeader("Accept", "application/json");

        // Format: "weather temperature=XX.XX,humidity=XX.X,pressure=XX.XX,..."
        String payload = String("weather ") + "temperature=" + String(temperature, 2) + ","
            + "humidity=" + String(humidity, 1) + "," + "pressure=" + String(pressure, 2) + ","
            + "illumination=" + String(illumination, 1) + "," + "dew_point=" + String(dewpoint, 1)
            + "," + "battery_voltage=" + String(battery_voltage, 2) + ","
            + "solar_panel_voltage=" + String(solar_panel_voltage, 2);

        int response_code = http.POST(payload);
        serial_log(payload);

        if (response_code > 0) {
            serial_log("HTTP Response Code: ");
            serial_log(String(response_code));
        } else {
            serial_log("Error in HTTP request: ");
            serial_log(String(response_code));
        }

        http.end();
    } else {
        serial_log("WiFi not connected");
    }
}

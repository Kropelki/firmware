#ifndef INFLUXDB_H
#define INFLUXDB_H

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
 * @param dew_point Dew point in Celsius
 * @param illumination Light intensity measurement
 * @param battery_voltage Battery voltage level
 * @param solar_panel_voltage Solar panel voltage output
 *
 * @note Requires active WiFi connection. Function will log error if WiFi disconnected.
 * @note Uses InfluxDB API v2 with token-based authentication.
 */
void send_to_influx_db(float temperature, float humidity, float pressure, float dew_point,
    float illumination, float battery_voltage, float solar_panel_voltage);

#endif // INFLUXDB_H

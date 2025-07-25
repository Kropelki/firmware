#ifndef INFLUXDB_H
#define INFLUXDB_H

#include "measurement.h"
#include "utils.h"

/**
 * Sends weather sensor data to InfluxDB using HTTP POST request
 *
 * This function constructs an InfluxDB line protocol payload with sensor readings
 * and transmits it to the configured InfluxDB instance via HTTP API v2.
 *
 * Link to InfluxDB API documentation:
 * https://docs.influxdata.com/influxdb3/cloud-serverless/get-started/write/?t=v2+API#write-line-protocol-to-influxdb
 *
 * @note Requires active WiFi connection. Function will log error if WiFi disconnected.
 * @note Uses InfluxDB API v2 with token-based authentication.
 */
void send_to_influx_db(const Measurement& measurement);

#endif // INFLUXDB_H

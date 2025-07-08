#ifndef INFLUXDB_H
#define INFLUXDB_H

#include "env.h"
#include "utils.h"

#include <HTTPClient.h>

void send_to_influx_db(float temperature, float humidity, float pressure, float dew_point,
    float illumination, float battery_voltage, float solar_panel_voltage);

#endif // INFLUXDB_H

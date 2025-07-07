#ifndef INFLUXDB_H
#define INFLUXDB_H

#include "env.h"
#include "utils.h"

#include <HTTPClient.h>

void sendToInfluxDB(float temperature_c, float humidity, float pressure, float dewpoint_c,
    float illumination, float battery_voltage, float solar_panel_voltage);

#endif // INFLUXDB_H

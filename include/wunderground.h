#ifndef WUNDERGROUND_H
#define WUNDERGROUND_H

#include "utils.h"

/**
 * Sends weather data to Weather Underground station
 *
 * This function transmits current weather measurements to the Weather Underground
 * service via HTTP GET request. The data is sent in Fahrenheit units for temperature
 * and dew point, with humidity as percentage and barometric pressure in inches.
 */
void send_to_wunderground(const Measurement& measurement);

#endif // WUNDERGROUND_H

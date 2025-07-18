#ifndef WUNDERGROUND_H
#define WUNDERGROUND_H

/**
 * Sends weather data to Weather Underground station
 *
 * This function transmits current weather measurements to the Weather Underground
 * service via HTTP GET request. The data is sent in Fahrenheit units for temperature
 * and dew point, with humidity as percentage and barometric pressure in inches.
 *
 * @param temperature Current temperature in Fahrenheit
 * @param humidity Relative humidity as percentage (0-100)
 * @param baromin Barometric pressure in inches of mercury
 * @param dew_point Dew point temperature in Fahrenheit
 */
void send_to_wunderground(float temperature, int humidity, float baromin, float dew_point);

#endif // WUNDERGROUND_H

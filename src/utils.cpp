#include "utils.h"

String logBuffer = "";

void serialLog(String message)
{
    logBuffer += message + "\n";
    Serial.println(message);
}

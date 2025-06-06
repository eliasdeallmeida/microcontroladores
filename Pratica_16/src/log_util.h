#ifndef LOG_UTIL_H
#define LOG_UTIL_H

#include <Arduino.h>

void logToFile(const String &message);
String getFormattedTime();
String readLastLinesOfLog(int qtd);

#endif

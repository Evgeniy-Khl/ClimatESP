#ifndef __LOGGER_H
#define __LOGGER_H

#include <Arduino.h>

void logEvent(const char* format, ...);
void logEvent(const String& message);

#endif

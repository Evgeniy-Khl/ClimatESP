#ifndef __LOGGER_H
#define __LOGGER_H

#include <Arduino.h>

extern bool fsMounted;

void logEvent(const char* format, ...);
void logEvent(const String& message);

#endif


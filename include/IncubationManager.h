#ifndef __INCUBATION_MANAGER_H
#define __INCUBATION_MANAGER_H

#include <Arduino.h>

class IncubationManager {
public:
    static void init();
    static void tick(); // Вызывается каждую секунду из newSecond()

private:
    static void processPID();
    static void processVentilation();
    static void processRotation();
    static void processAlarm();
};

#endif

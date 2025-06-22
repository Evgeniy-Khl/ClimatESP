#include "main.h"
#include "sensors.h"

#define TUNING	170

void temperature_check(void){
  char buff[20];
  DeviceAddress sensorAddress;        // Переменная для хранения адреса датчика
  for (uint8_t i = 0; i < numberOfDevices;){
    float tempC = sensors.getTempCByIndex(i);
    sprintf(buff, "TempCByIndex(%i): %5.1f °C",i,tempC);
    Serial.println(buff);

    if(tempC == DEVICE_DISCONNECTED_C) ds[i].err++;
    else ds[i].pvT = tempC * 10;
    //----- Коректировка датчика DS18B20 ---------
    sensors.getAddress(sensorAddress, i);
    uint8_t alarmH = sensors.getHighAlarmTemp(sensorAddress);
    sprintf(buff, "HighAlarmTemp(%i): %3i",i,alarmH);
    Serial.println(buff);
    if(alarmH==TUNING){
      uint8_t alarmL = sensors.getLowAlarmTemp(sensorAddress);
      ds[i].pvT += alarmL;
    }
  }
  sensors.requestTemperatures();
}
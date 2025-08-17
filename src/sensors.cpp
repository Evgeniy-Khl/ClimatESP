#include "main.h"

#define TUNING	170

void sensorType(){
  MYDEBUG_PRINTLN("Определение типа датчика...");
  // 1. Пытаемся найти датчик DS18B20. Это более надежная проверка.
  sensors.begin(); // Инициализируем шину 1-Wire
  // numberOfDevices = sensors.getDeviceCount();
  numberOfDevices = sensors.getDS18Count();
  if(numberOfDevices > 0) {
      detectedSensor = SENSOR_DS18B20;
      if(numberOfDevices > MAX_DEVICE) numberOfDevices = MAX_DEVICE;
      MYDEBUG_PRINT("Обнаружен датчик DS18B20:"); MYDEBUG_PRINT(numberOfDevices, DEC); MYDEBUG_PRINTLN(" шт.");
      sensors.setWaitForConversion(false);    // false: функция вернет управление немедленно.
      sensors.setCheckForConversion(false);   // Часто используется вместе с waitForConversion = false
      sensors.setAutoSaveScratchPad(false);   // Флаг автоматического сохранения настроек в EEPROM датчика.
      sensors.setResolution(12);// Устанавливаем разрешение для всех датчиков (9, 10, 11, or 12 бит)
      sensors.requestTemperatures(); // Отправляем команду на измерение
      //------- Получаем и сохраняем адреса всех найденных датчиков ------
      for (int i = 0; i < numberOfDevices; i++){
        if(sensors.getAddress(sensorAddresses[i], i)){
          DEBUG_PRINTF("  Датчик %d: ", i);
          printAddress(sensorAddresses[i]);
          MYDEBUG_PRINTLN();
        } else {
          DEBUG_PRINTF("Не удалось получить адрес для датчика %d\n", i);
        }
      }
   } else {
      // 2. Если DS18B20 не найден, пытаемся прочитать данные с DHT22.
      delay(1000);
      dht.begin(); // Инициализируем датчик DHT
      // Делаем тестовое чтение. Если результат не "NaN", значит, это DHT.
      if (!isnan(dht.readTemperature())) {
        detectedSensor = SENSOR_DHT22;
        MYDEBUG_PRINTLN("Обнаружен датчик: DHT22");
      }
   }
}

void sensorCheck(){
  switch (detectedSensor){
    case SENSOR_DHT22:{ // <--- Открывающая скобка
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      if (isnan(h) || isnan(t)) {
        MYDEBUG_PRINTLN("Ошибка чтения с DHT22!");
        if(++ds[0].errDevice > 5) {ds[0].pvT = 126; ds[1].pvT = 126; ds[0].errDevice = 5;}
      } else {
        ds[0].errDevice = 0;
        ds[0].pvT = round(t);
        ds[1].pvT = round(h);
        MYDEBUG_PRINT("t= "); MYDEBUG_PRINT(t); MYDEBUG_PRINTLN(" °C");
        MYDEBUG_PRINT("RH= "); MYDEBUG_PRINT(h); MYDEBUG_PRINT(" %\t");
      }
      break;
    }
    case SENSOR_DS18B20: checkDs18b20(); break;
    case UNKNOWN: MYDEBUG_PRINTLN("Датчики не подключены!"); break;
  }
}

//------------- индикация 66,0 - завис датчик. --------------
bool check_freeze(uint8_t i){
 if(ds[i].pvT == ds[i].previousValue){
    if(++ds[i].froze> 600){ds[i].froze = 600; return true;}
 } else {ds[i].froze = 0; ds[i].previousValue = ds[i].pvT;}
 return false;
}

void temperature_check(void){
#ifdef DEBUG
  char buff[100];
#endif
  DeviceAddress sensorAddress;        // Переменная для хранения адреса датчика
  for (uint8_t i = 0; i < numberOfDevices; i++){
    float tempC = sensors.getTempCByIndex(i);
    DEBUG_SPRINTF(buff, "TempCByIndex(%i): %5.1f °C",i,tempC);
    MYDEBUG_PRINTLN(buff);
    if(tempC == DEVICE_DISCONNECTED_C) {
      ds[i].errDevice++;
      if(ds[i].errDevice > 5) {ds[i].pvT = 1990; ds[i].errDevice = 5;}
    }
    else {
      ds[i].pvT = tempC * 10;
      ds[i].errDevice = 0;
    }
    //----- Коректировка датчика DS18B20 ---------
    sensors.getAddress(sensorAddress, i);
    uint8_t alarmH = sensors.getHighAlarmTemp(sensorAddress);
    DEBUG_SPRINTF(buff, "HighAlarmTemp(%i): %3i",i,alarmH);
    MYDEBUG_PRINTLN(buff);
    if(alarmH==TUNING){
      uint8_t alarmL = sensors.getLowAlarmTemp(sensorAddress);
      ds[i].pvT += alarmL;
    }
    if(check_freeze(i)){
      ds[i].pvT = 660;    // индикация 66,0 - завис датчик.
      FROZE = 1;
    }
  }
  sensors.requestTemperatures();
}

#define V_REF   	5 //?????????????????????????????????????????????????????
#define ADC_RESOLUTION	512 //?????????????????????????????????????????????????????????????????????
// для HIH-5030
// Vadc бинарное значение ADC -> в десятичное значение относительной влажности (%)
int16_t valDcToRH(uint16_t Vadc){
 float tmpRH, tmpK;
  tmpRH = (float)Vadc/ADC_RESOLUTION; //????????????????????????????????????????????????????????????????
  tmpRH -= 0.1515; tmpRH /= 0.00636;
  if(ds[0].pvT<850){tmpK = 0.00216 * ds[0].pvT/10; tmpK = 1.0546 - tmpK;}// корекция по температуре
  else tmpK=1;      
  tmpRH /= tmpK;
  tmpRH *= 10;
  tmpRH += settings.sp_structs[0].spRH;             //sp[0].spRH->ПОДСТРОЙКА HIH-5030!!
  if (tmpRH>1000) tmpRH=1000; else if (tmpRH<0) tmpRH=0;
  return tmpRH;
}

/* int16_t lowPassF2(int16_t PV)
{
float val;
  // val = A1*PVold1-A2*PVold2+A3*PV;
  // PVold2 = PVold1;
  // PVold1 = val;
  return val;
}; */

/* case 14:                               // У14 подстройка датчика DS18B20
if(devices==1)
{ 
  try = 0;
  do {
    w1_init(); w1_write(0xCC);// 1 Wire Bus initialization; Skip ROM [CCH] command
    w1_write(0xBE); // Read Scratchpad command [BE]
    ptr_to_ram = ds_buffer;
    for (byte=0; byte < 8; byte++){*ptr_to_ram++ = w1_readnew();}
    crc = w1_readnew(); // Read CRC byte
    ptr_to_ram = ds_buffer;
    if(w1_dow_crc8(ptr_to_ram, 8)==crc){try = 2; if(ds_buffer[2]==TUNING) editBuff=(signed char)ds_buffer[3]; else editBuff=0;}
  } while (++try < 2);
} else editBuff=999; break;*/
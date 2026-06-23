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
          DEBUG_PRINTF("He удалось получить адрес для датчика %d\n", i);
        }
      }
      // Инициализируем расчет относительной влажности для HIH-5030
      // При старте передаем 25 градусов, так как DS18B20 еще не завершил конвертацию
      if(getRelativeHumidityESP8266(25.0) > 10){
        HIH5030 = 1; MYDEBUG_PRINTLN("Обнаружен датчик: HIH-5030");
      } else HIH5030 = 0;
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
        MYDEBUG_PRINTLN("Ошибка чтения c DHT22!");
        if(++ds[0].errDevice > 5) {ds[0].pvT = 126; ds[1].pvT = 126; ds[0].errDevice = 5;}
      } else {
        ds[0].errDevice = 0;
        ds[0].pvT = round(t * 10);
        pvRH = round(h);
        MYDEBUG_PRINT("t= "); MYDEBUG_PRINT(t); MYDEBUG_PRINTLN(" °C");
        MYDEBUG_PRINT("RH= "); MYDEBUG_PRINT(h); MYDEBUG_PRINT(" %\t");
      }
      break;
    }
    case SENSOR_DS18B20: checkDs18b20(); break;
    case UNKNOWN: MYDEBUG_PRINTLN("Датчики не подключены!"); break;
  }
}

int16_t checkPV(uint8_t cn){
  int16_t err;
  if(cn==1 && HIH5030){
     if(pvRH < 10) {errorsFlag.value |= (cn+1); err = 0;}
     else err = settings.sp_structs[1].spRH - pvRH;
     ds[1].pvErr = err;         // err > 0 -> холодно
  } else {
     if(ds[cn].pvT >= 850) {errorsFlag.value |= (cn+1); err = 0;}
     else err = settings.sp_structs[cn].spT - ds[cn].pvT;
     ds[cn].pvErr = err;        // err > 0 -> холодно
  };
  return err;
}

//------------- индикация 66,0 - завис датчик. --------------
bool check_freeze(uint8_t i){
 if(ds[i].pvT == ds[i].previousValue){
    if(++ds[i].froze> 600){ds[i].froze = 600; return true;}
 } else {ds[i].froze = 0; ds[i].previousValue = ds[i].pvT;}
 return false;
}

void checkDs18b20(void){
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
      if(ds[i].errDevice > 5) {ds[i].pvT = SENSOR_ERROR_VAL; ds[i].errDevice = 5;}
    }
    else {
      ds[i].pvT = round(tempC * 10);
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
      ds[i].pvT = SENSOR_FROZEN_VAL;    // индикация 66,0 - завис датчик.
      FROZE = 1;
    }
  }
  sensors.requestTemperatures();

  if(HIH5030){
    pvRH = getRelativeHumidityESP8266((float)ds[0].pvT / 10.0); // Передаем реальную температуру
    if (pvRH > 100.0 || pvRH < 0) pvRH = 255;           // Ограничиваем диапазон [0% ... 100%]
  } else if (numberOfDevices == 2){
    uint8_t valTable = tableRH(ds[0].pvT, ds[1].pvT);   // если отсутствует HIH то ...
    if(valTable == 255) pvRH = 255;
    else if(valTable > 100) pvRH = 100;
    else pvRH = valTable;
  }
}

/**
 * Вычисляет относительную влажность для датчика HIH-5030 на ESP8266.
 * 
 * @param temperature  Текущая температура окружающей среды в °C.
 * @return             Относительная влажность в % RH.
 */
int16_t getRelativeHumidityESP8266(float temperature) {
    // 1. Считываем значение с АЦП ESP8266 (всегда 10 бит: 0 - 1023)
    int adcValue = analogRead(A0);
    // 2. Переводим значение АЦП в напряжение.
    
    float vOut = (adcValue / 1023.0) * 3.3;                     // максимальное входное напряжение на пине A0 составляет 3.3V.
    // 3. Расчет влажности без учета температуры (из даташита HIH-5030)
    float sensorRH = ((vOut / 3.3) - 0.1515) / 0.00636;         // Формула: RH = ((Vout / Vsupply) - 0.1515) / 0.00636
    // 4. Температурная компенсация (из даташита)
    float trueRH = sensorRH / (1.0546 - (0.00216 * temperature));// Формула: True RH = Sensor RH / (1.0546 - 0.00216 * T)
    trueRH += settings.sp_structs[0].spRH;                      //sp[0].spRH->ПОДСТРОЙКА HIH-5030!!
    return trueRH;
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
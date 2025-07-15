#include "main.h"

#define UNALTERED   2 // неизменный

void beeperOn(uint8_t val){
  beepOn = val;
  digitalWrite(BEEP_PIN, LOW); // Включаем бипер
}

void PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki) {
    pid->Kp = (float)Kp/4;
    pid->Ki = (float)Ki/10000;
}

int16_t UpdatePID(uint8_t cn){
  int16_t error, max = 255, min = -127;
  // float output;
  if(settings.sp_structs[0].mode == 4 && cn == 1){  // 4-импульсный режим для канала №2
    max = settings.sp_structs[1].pulse * 1000 / 2; 
    min = -max / 2;
  }
  // Вычисление ошибки
  error = settings.sp_structs[cn].spT - ds[cn].pvT;
  ds[cn].pvErr = error;         // error > 0 -> холодно
  // Пропорциональная составляющая
  pid[cn].pPart = (float)error * pid[cn].Kp;
  // Интегральная составляющая
  pid[cn].iPart += (float)error * pid[cn].Ki;// * dt;
  // Ограничение выходного значения и антивиндовинг
  if (pid[cn].pPart >= max) pid[cn].iPart = 0; // Сброс интеграла
  else if (pid[cn].pPart <= min) pid[cn].iPart = 0; // Сброс интеграла
  // Суммарное управляющее воздействие
  pid[cn].output = pid[cn].pPart + pid[cn].iPart;
  if(pid[cn].output < 0) pid[cn].output = 0;
  // Serial.print("Current value of pid[cn].output before cast: ");
  // Serial.println(pid[cn].output); // Эта строка покажет вам реальное значение переменной
  error = (int16_t)pid[cn].output;
  // Serial.print("Value of error after cast: ");
  // Serial.println(error);
  return error;
}
//------------- симистричный таймер -------------------
void rotate_trays(void){
  if(!TURN){
    if(--pvTimer == 0){
      pvTimer = settings.sp_structs[0].timer; 
      TURN = PCF_OFF;
      DEBUG_PRINTLN("TURN = PCF_OFF");
    }
  } else {
    if(--pvTimer == 0){
      if(settings.sp_structs[1].timer) pvTimer = settings.sp_structs[1].timer;
      else pvTimer = settings.sp_structs[0].timer;
      TURN = PCF_ON;
      DEBUG_PRINTLN("TURN = PCF_ON");
    }
  }
}

//------------- индикация 66,0 - завис датчик. --------------
bool check_freeze(uint8_t i){
 if(ds[i].pvT == ds[i].previousValue){
    if(++ds[i].duration > 600){ds[i].duration = 600; return true;}
 } else {ds[i].duration = 0; ds[i].previousValue = ds[i].pvT;}
 return false;
}

int16_t checkPV(uint8_t cn){
  int16_t err;
  if(cn==1 && HIH5030){
     if(pvVadcRH < 80) {errorsFlag.value |= (cn+1); err = 0;}
     else err = settings.sp_structs[1].spRH - pvRH;
     ds[1].pvErr = err;         // err > 0 -> холодно
  } else {
     if(ds[cn].pvT >= 850) {errorsFlag.value |= (cn+1); err = 0;}
     else err = settings.sp_structs[cn].spT - ds[cn].pvT;
     ds[cn].pvErr = err;        // err > 0 -> холодно
  };
  return err;
}

uint8_t RelayPos(unsigned char cn, unsigned char hysteresis){	// [n] канал № 1 или 2
  uint8_t x=UNALTERED;
  int16_t err = checkPV(cn);        // err > 0 -> холодно
  if(err >= hysteresis) x = ON;    // включить
  if(err <= 0) x = OFF;            // отключить
  return x;
}

uint8_t RelayNeg(uint8_t cn, uint8_t on, uint8_t off){	// [n] канал № 1 или 2
  uint8_t x=UNALTERED;
  int16_t err = checkPV(cn);        // err > 0 -> холодно
  if ((err+on) <= 0) x = ON;        // включить
  if ((err+off) >= 0) x = OFF;      // отключить
  return x;
}

void OutPulse(void){
  int16_t err = checkPV(1);                     // err > 0 -> холодно
  uint16_t maxPulse = settings.sp_structs[1].pulse * 1000 / 2;// длительность впрыска не должна превышать пол периода
  if(err == 0){pvPulse = 0; return;};
  if(ds[0].pvErr >= settings.sp_structs[0].alarm){pvPulse = 0; return;};          // отключение впрыска по 2 каналу если идет разогрев
  pvPulse = UpdatePID(1);                       // определение длительности ВКЛ. состояния
  if(pvPulse < settings.sp_structs[0].pulse) pvPulse = settings.sp_structs[0].pulse;
  else if(pvPulse > maxPulse) pvPulse = maxPulse;   // длит. впрыска не должна превыщать длит.переода
  if(ds[1].pvErr < 0) pvPulse = 0;                  // отключение впрыска по 2 каналу если перелив
}

void OutStatusLed(void){
    for(uint8_t i = 0; i < 6; i++){
      uint8_t numBit = 1 << i;
      dataLed[i] = (~portOut.value) & numBit;
    }
}

uint8_t checkSetpoint(void){
  uint8_t err = 0;
  //--------- Загрузка конфигурации --------------------------------------------
  if(LittleFS.exists("/setpoint.json")){
      if(!loadConfig()){
        DEBUG_PRINTLN("Конфігурація не завантажена!");
        err = 1 ;
        saveConfig();  // значения по умолчанию
      }
  } else {
      saveConfig();  // значения по умолчанию
      DEBUG_PRINTLN("Конфігурація за замовчуванням!");
      err = 2 ;
  }
  DEBUG_PRINTLN("\n>> Итоговые значения после загрузки из FS:");
  #ifdef DEBUG
    printConfig();
  #endif
  return err;
}

uint8_t checkConfig(void){
  uint8_t err = 0;
  if(LittleFS.exists("/config.json")){
    //file exists, reading and loading
    DEBUG_PRINTLN("reading config file");
    File configFile = LittleFS.open("/config.json", "r");
    if(configFile){
      DEBUG_PRINTLN("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      JsonDocument json;
      auto deserializeError = deserializeJson(json, buf.get());
      serializeJson(json, Serial);
      if( ! deserializeError ){
        DEBUG_PRINTLN("\nparsed json");
        strcpy(botToken, json["botToken"]);
        strcpy(chatID, json["chatID"]);
      } else {
        DEBUG_PRINTLN("failed to load json config");
        err = 3;
      }
      configFile.close();
    } else {
      err = 2;
    }
  } else {
    err = 1;
  }
  return err;
}

//-------- Функция для печати текущих значений структуры в Serial порт --------
#ifdef DEBUG
void printConfig() {
    DEBUG_PRINTLN("--------------------");
    for (int i = 0; i < 2; i++) {
        DEBUG_PRINTF("Элемент settings.sp_structs[%d]:\n", i);
        DEBUG_PRINTF("  spT: %d\n", settings.sp_structs[i].spT);
        DEBUG_PRINTF("  spRH: %d\n", settings.sp_structs[i].spRH);
        DEBUG_PRINTF("  alarm: %d\n", settings.sp_structs[i].alarm);
        DEBUG_PRINTF("  coolOn: %d\n", settings.sp_structs[i].coolOn);
        DEBUG_PRINTF("  coolOff: %d\n", settings.sp_structs[i].coolOff);
        DEBUG_PRINTF("  timer: %d\n", settings.sp_structs[i].timer);
        DEBUG_PRINTF("  aeration: %d\n", settings.sp_structs[i].aeration);
        DEBUG_PRINTF("  auxiliary: %d\n", settings.sp_structs[i].auxiliary);
        DEBUG_PRINTF("  flapLimit: %d\n", settings.sp_structs[i].flapLimit);
        DEBUG_PRINTF("  state: %d\n", settings.sp_structs[i].state);
        DEBUG_PRINTF("  pulse: %d\n", settings.sp_structs[i].pulse);
        DEBUG_PRINTF("  mode: %d\n", settings.sp_structs[i].mode);
        DEBUG_PRINTF("  extendMode: %d\n", settings.sp_structs[i].extendMode);
        DEBUG_PRINTF("  Kp: %d\n", settings.sp_structs[i].Kp);
        DEBUG_PRINTF("  Ki: %d\n", settings.sp_structs[i].Ki);
    }
    DEBUG_PRINTLN("--------------------");
}
#endif

//----------- Функция сохранения конфигурации в JSON файл ----------------
void saveConfig() {
    DEBUG_PRINTLN("Сохранение конфигурации...");

    // Создаем JSON документ. Размер 512 байт более чем достаточен.
    JsonDocument doc;

    // Создаем корневой JSON массив
    JsonArray jsonArray = doc.to<JsonArray>();

    // Проходим по массиву структур и добавляем данные в JSON
    for (int i = 0; i < 2; i++) {
        JsonObject obj = jsonArray.add<JsonObject>();
        obj["spT"] = settings.sp_structs[i].spT;
        obj["spRH"] = settings.sp_structs[i].spRH;
        obj["alarm"] = settings.sp_structs[i].alarm;
        obj["coolOn"] = settings.sp_structs[i].coolOn;
        obj["coolOff"] = settings.sp_structs[i].coolOff;
        obj["timer"] = settings.sp_structs[i].timer;
        obj["aeration"] = settings.sp_structs[i].aeration;
        obj["auxiliary"] = settings.sp_structs[i].auxiliary;
        obj["flapLimit"] = settings.sp_structs[i].flapLimit;
        obj["state"] = settings.sp_structs[i].state;
        obj["pulse"] = settings.sp_structs[i].pulse;
        obj["mode"] = settings.sp_structs[i].mode;
        obj["extendMode"] = settings.sp_structs[i].extendMode;
        obj["Kp"] = settings.sp_structs[i].Kp;
        obj["Ki"] = settings.sp_structs[i].Ki;
    }

    // Открываем файл для записи
    File configFile = LittleFS.open("/setpoint.json", "w");
    if (!configFile) {
        DEBUG_PRINTLN("Не удалось открыть файл для записи");
        return;
    }

    // Сериализуем JSON в файл
    if (serializeJson(doc, configFile) == 0) {
        DEBUG_PRINTLN("Ошибка записи в файл");
    } else {
        DEBUG_PRINTLN("Конфигурация успешно сохранена.");
    }
    
    configFile.close();
}

//------------ Функция загрузки конфигурации из JSON файла -------------
bool loadConfig() {
    DEBUG_PRINTLN("Загрузка конфигурации...");

    // Открываем файл для чтения
    File configFile = LittleFS.open("/setpoint.json", "r");
    if (!configFile) {
        DEBUG_PRINTLN("Не удалось открыть файл для чтения. Используются значения по умолчанию.");
        return false;
    }

    // Создаем JSON документ для десериализации
    JsonDocument doc;

    // Десериализуем JSON из файла
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        DEBUG_PRINT("Ошибка десериализации JSON: ");
        DEBUG_PRINTLN(error.c_str());
        configFile.close();
        return false;
    }
// Закрываем файл после чтения
    configFile.close();

    // Получаем корневой JSON массив
    JsonArray jsonArray = doc.as<JsonArray>();

// spT spRH timer alarm coolOn coolOff aeration flapLimit state service pulse mode extendMode Kp Ki Kd
    // Проходим по JSON массиву и заполняем структуру
    int i = 0;
    for (JsonObject obj : jsonArray) {
        if (i < 2) {
            settings.sp_structs[i].spT = obj["spT"];
            settings.sp_structs[i].spRH = obj["spRH"];
            settings.sp_structs[i].alarm = obj["alarm"];
            settings.sp_structs[i].coolOn = obj["coolOn"];
            settings.sp_structs[i].coolOff = obj["coolOff"];
            settings.sp_structs[i].timer = obj["timer"];
            settings.sp_structs[i].aeration = obj["aeration"];
            settings.sp_structs[i].auxiliary = obj["auxiliary"];
            settings.sp_structs[i].flapLimit = obj["flapLimit"];
            settings.sp_structs[i].state = obj["state"];
            settings.sp_structs[i].pulse = obj["pulse"];
            settings.sp_structs[i].mode = obj["mode"];
            settings.sp_structs[i].extendMode = obj["extendMode"];
            settings.sp_structs[i].Kp = obj["Kp"];
            settings.sp_structs[i].Ki = obj["Ki"];
            i++;
        }
    }
    DEBUG_PRINTLN("Конфигурация успешно загружена.");
    return true;
}

#ifdef DEBUG
// Вспомогательная функция для вывода адреса датчика
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) DEBUG_PRINT("0");
    DEBUG_PRINT(deviceAddress[i], HEX);
    if (i < 7) DEBUG_PRINT(":");
  }
}
#endif

uint8_t tableRH(int16_t maxT, int16_t minT){
  int16_t dT = 255;
  if (maxT>199 && maxT<410){ // maxT> 19.9 и maxT< 41.0
     dT = (maxT-minT)*16/10;
     if (dT<0) dT = 240;        // задаем число при котором dT >>=3; выполняется -> dT>20
     maxT /=10;
     dT >>=3;
     if (dT>20) dT = 255;
     else if (dT==0) dT = 100;
     else {maxT -= 20; maxT *= 20; maxT += (dT-1); dT = tabRH[maxT];};
  }
  return dT;
}

/*
errors = 0x01   // ОШИБКА ДАТЧИКА 0  199-потерян; 66,0-завис [E01]
errors = 0x02   // ОШИБКА ДАТЧИКА 1  199-потерян; 66,0-завис [E02]
errors = 0x04   // ОТКЛОНЕНИЕ КАНАЛ 0 [E04]
errors = 0x08   // ОТКЛОНЕНИЕ КАНАЛ 1 [E08]
errors = 0x10   // отказ одного из двух датчиков температуры
errors = 0x20   // отказ вспомогательного датчика температуры
errors = 0x40   // ПЕРЕГРЕВ СИМИСТОРА ! [ПГ]
*/
uint8_t alarm(void){
  uint8_t cn;
  int16_t err, above, lower;
  for (cn=0; cn<2; cn++){
    lower = settings.sp_structs[cn].alarm;          // ниже
    above = lower;                                  // выше
    // above += sp[cn].offSet;                      // если режим ОХЛАЖДЕНИЕ или ОСУШЕНИЕ
    err = ds[cn].pvErr;
    if(abs(err) < lower) ds[cn].deviation = 1;      // вышли на заданную температуру
    if(ds[0].deviation == 0) ds[1].deviation = 0;   // отключение тревоги по 2 каналу
    if(ds[cn].deviation){
      if (err > lower){                             // ПЕРЕОХЛАЖДЕНИЕ
          ds[cn].deviation = 2;                     // мигают цифры
          errorsFlag.value |= ((cn+1)<<2);          // включить сигнал АВАРИЯ
      }
    };
    if (err < -above){                              // ПЕРЕГРЕВ
        ds[cn].deviation = 3;                       // мигают цифры
        errorsFlag.value |= ((cn+1)<<2);            // включить сигнал АВАРИЯ
    };
  };
  cn = OFF;   
  if(errorsFlag.value){
    if(errorsFlag.value & 0x03) lower = 100;
    else lower = 50;
    if(disableBeep==0) {beeperOn(lower); cn = ON;};// длительность звукового сигнала и включить канал 4 (6 А)
  }
  else disableBeep = 0;
  return cn;
}

// // Вспомогательная функция для печати
// void printBinary(unsigned char byte) {
//   for (int i = 7; i >= 0; i--) {
//     DEBUG_PRINTLN(bitRead(byte, i));
//   }
// }

void reset(void){
  settings.sp_structs[0].spT = SPT_0;
  settings.sp_structs[0].spRH = SPRH_0;
  settings.sp_structs[0].alarm = ALARM_0;
  settings.sp_structs[0].coolOn = COOLON_0;
  settings.sp_structs[0].coolOff = COOLOFF_0;
  settings.sp_structs[0].timer = TIMER_0;
  settings.sp_structs[0].aeration = AERATION_0;
  settings.sp_structs[0].auxiliary = AUXILIARY_0;
  settings.sp_structs[0].flapLimit = FLPCLOSE;
  settings.sp_structs[0].state = STATE_0;
  settings.sp_structs[0].pulse = PULSE_0;
  settings.sp_structs[0].mode = MODE_0;
  settings.sp_structs[0].extendMode = EXTMODE_0;
  settings.sp_structs[0].Kp = KP_0_1;
  settings.sp_structs[0].Ki = KI_0_1;

  settings.sp_structs[1].spT = SPT_1;
  settings.sp_structs[1].spRH = SPRH_1;
  settings.sp_structs[1].alarm = ALARM_1;
  settings.sp_structs[1].coolOn = COOLON_1;
  settings.sp_structs[1].coolOff = COOLOFF_1;
  settings.sp_structs[1].timer = TIMER_1;
  settings.sp_structs[1].aeration = AERATION_1;
  settings.sp_structs[1].auxiliary = AUXILIARY_1;
  settings.sp_structs[1].flapLimit = FLPOPEN;
  settings.sp_structs[1].state = STATE_1;
  settings.sp_structs[1].pulse = PULSE_1;
  settings.sp_structs[1].mode = MODE_1;
  settings.sp_structs[1].extendMode = EXTMODE_1;
  settings.sp_structs[1].Kp = KP_0_1;
  settings.sp_structs[1].Ki = KI_0_1;

  for (uint8_t i = 0; i < 8; i++) { data[i] = TOP;}
  module.setDisplay(data, 8); // Вывод на дисплей "--- --- --"
  delay(500);
  for (uint8_t i = 0; i < 8; i++) { data[i] = DEF;}
  module.setDisplay(data, 8); // Вывод на дисплей "--- --- --"
  delay(500);
  for (uint8_t i = 0; i < 8; i++) { data[i] = BOT;}
  module.setDisplay(data, 8); // Вывод на дисплей "--- --- --"
  saveConfig();
  delay(3000);
}

//============================== Config ========================================
void initEnvironment(void){
#ifdef DEBUG
  char displStr[65];
#endif
  //--------- инициализация PID --------------------------------------------
  PID_Init(&pid[0], settings.sp_structs[0].Kp, settings.sp_structs[0].Ki);
  PID_Init(&pid[1], settings.sp_structs[1].Kp, settings.sp_structs[1].Ki);

  DEBUG_SPRINTF(displStr,"Пропорц.0= %g  Ітеграл.0= %g", pid[0].Kp,pid[0].Ki);
  DEBUG_PRINTLN(displStr);
  DEBUG_SPRINTF(displStr,"Пропорц.1= %g  Ітеграл.1= %g", pid[1].Kp,pid[1].Ki);
  DEBUG_PRINTLN(displStr);
  
  //------------------------------------------------------------------------
  /* DEBUG_PRINTLN("\n");
  uint32_t realSize = ESP.getFlashChipRealSize(); // Получаем реальный размер flash
  uint32_t ideSize = ESP.getFlashChipSize();    // Получаем размер, установленный в IDE
  FlashMode_t ideMode = ESP.getFlashChipMode();

  DEBUG_PRINTF("Flash real id:   %08X\n", ESP.getFlashChipId());
  DEBUG_PRINTF("Flash real size: %u bytes\n\n", realSize);

  DEBUG_PRINTF("Flash ide  size: %u bytes\n", ideSize);
  DEBUG_PRINTF("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
  DEBUG_PRINTF("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

  if (ideSize != realSize) {
    DEBUG_PRINTLN("Внимание! Размер Flash, установленный в IDE, не совпадает с реальным!");
  } else {
    DEBUG_PRINTLN("Размер Flash в IDE совпадает с реальным.");
  }
  DEBUG_PRINTLN(); */

/* 
  //---------- Изменяем яркость светодиода ----------------------------------------
  // Пин, к которому подключен светодиод (GPIO2)
  pinMode(LEDPIN, OUTPUT);    // Устанавливаем пин светодиода как выход
  // Можно установить желаемую частоту ШИМ (опционально)
  // analogWriteFreq(1000);   // По умолчанию и так 1000 Гц
  // Можно установить желаемый диапазон (опционально)
  analogWriteRange(255);      // Если хотите диапазон 0-255
  //===============================================================================
 */

  
  // Wire.begin(D2, D1);      // Если вы хотите использовать другие пины для I2C (например, D2 для SDA, D1 для SCL)
  //--------------------- Инициализация PCF8574 ----------------------------------
  /* Пример: Установить все пины PCF8574 как выходы и выключить их (записать 0)
            Для PCF8574, чтобы использовать пин как "выход", мы просто записываем в него значение.
            Чтобы использовать пин как "вход", мы записываем в него '1' (высокий уровень),
            а затем читаем состояние. Внутренние подтягивающие резисторы слабые. 
  */

  //---------- Инициализация DS3231 ----------------------------------------
  if(rtc.begin()) RTCENABLE = 1;
  //------------------------------------------------------------------------------
  // testAT24C32();              // тест
  // tft.drawString("AT24C32 test complete.", xpos, ypos, 2);
  // xpos = 0; ypos += 20;
  //==============================================================================

  //------------ Инициализация библиотеки DallasTemperature -----------------------------
  sensors.begin();
  sensors.setWaitForConversion(false);    // false: функция вернет управление немедленно.
  sensors.setCheckForConversion(false);   // Часто используется вместе с waitForConversion = false
  sensors.setAutoSaveScratchPad(false);   // Флаг автоматического сохранения настроек в EEPROM датчика.
  sensors.setResolution(12);// Устанавливаем разрешение для всех датчиков (9, 10, 11, or 12 бит)

  // Поиск устройств на шине 1-Wire
  numberOfDevices = sensors.getDeviceCount();
  if(numberOfDevices > MAX_DEVICE) numberOfDevices = MAX_DEVICE;
  DEBUG_PRINT("Found ");
  DEBUG_PRINT(numberOfDevices, DEC);
  DEBUG_PRINTLN(" devices.");
  
  #ifdef DEBUG
    if (numberOfDevices == 0) {
      DEBUG_PRINTLN("No DS18B20 sensors found! Check wiring and pull-up resistor.");
      // Можно остановить выполнение, если датчики не найдены
      // while(true) delay(100);
    } else {
      sensors.requestTemperatures(); // Отправляем команду на измерение
      DeviceAddress sensorAddress;
      DEBUG_PRINTLN("Sensor addresses:");
      // Выводим адрес каждого найденного устройства
      for (uint8_t i = 0; i < numberOfDevices; i++) {
        if (sensors.getAddress(sensorAddress, i)) {
          DEBUG_PRINT("  Sensor ");
          DEBUG_PRINT(i);
          DEBUG_PRINT(": ");
          printAddress(sensorAddress);
          DEBUG_PRINTLN();
        } else {
          DEBUG_PRINT("Could not get address for sensor ");
          DEBUG_PRINTLN(i);
        }
      }
    }
  #endif
  //==================================================================================
}
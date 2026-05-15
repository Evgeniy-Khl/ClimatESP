#include "main.h"

#define UNALTERED   2 // неизменный

void beeperOn(uint8_t val){
  beepOn = val;
  digitalWrite(BEEP_PIN, LOW); // Включаем бипер
}

void PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki) {
    pid->Kp = (float)Kp/5;
    pid->Ki = (float)Ki/10000;
}

int16_t UpdatePID(uint8_t cn){
  int16_t error, max = TRIACON * 2, min = -max;         // 255 * 2 = 510 -> 200 %
  // float output;
  if(settings.sp_structs[0].mode == 4 && cn == 1){  // 4-импульсный режим для канала №2
    max = settings.sp_structs[1].pulse * TRIACON / 2;  // 255 * 10 = 2550 -> 10 секунд
    min = -max;
  }
  // Вычисление ошибки
  error = checkPV(cn);
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
  // MYDEBUG_PRINT("Current value of pid[cn].output before cast: ");
  // MYDEBUG_PRINTLN(pid[cn].output); // Эта строка покажет вам реальное значение переменной
  error = (int16_t)pid[cn].output;
  // DEBUG_PRINTF("MAX=%d; MIN=%d;  ",max,min);
  // DEBUG_PRINTF("OUTPUT cn=%d: %3d%%\n",cn,error);
  return error;
}
//------------- симистричный таймер -------------------
void rotate_trays(void){
  if(!TURN){
    if(--pvTimer == 0){
      pvTimer = settings.sp_structs[0].timer; 
      TURN = PCF_OFF;
      MYDEBUG_PRINTLN("TURN = PCF_OFF");
    }
  } else {
    if(--pvTimer == 0){
      if(settings.sp_structs[1].timer) pvTimer = settings.sp_structs[1].timer;
      else pvTimer = settings.sp_structs[0].timer;
      TURN = PCF_ON;
      MYDEBUG_PRINTLN("TURN = PCF_ON");
    }
  }
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
  uint16_t maxPulse = settings.sp_structs[1].pulse * TRIACON / 2;// длительность впрыска не должна превышать пол периода
  if(err == 0){pvPulse = 0; return;};
  if(ds[0].pvErr >= settings.sp_structs[0].alarm){pvPulse = 0; return;};          // отключение впрыска по 2 каналу если идет разогрев
  pvPulse = UpdatePID(1);                       // определение длительности ВКЛ. состояния
  if(pvPulse < settings.sp_structs[0].pulse) pvPulse = settings.sp_structs[0].pulse;
  else if(pvPulse > maxPulse) pvPulse = maxPulse;   // длит. впрыска не должна превыщать длит.переода
  if(ds[1].pvErr < 0) pvPulse = 0;                  // отключение впрыска по 2 каналу если перелив
}

//-- для HTML страницы --
void OutStatusLed(void){
    for(uint8_t i = 0; i < 5; i++){
      uint8_t numBit = 1 << i;
      dataLed[i] = portOut.value & numBit;                          // если 1 -> OFF
      if(pctHeater <= 5) dataLed[1] = 1; else dataLed[1] = 0;      // НАГРЕВАТЕЛЬ
      if(pctHimidifier <= 5) dataLed[2] = 1; else dataLed[2] = 0;  // УВЛАЖНИТЕЛЬ
      if(errorsFlag.value) dataLed[5] = 0; else dataLed[5] = 1;     // если 1 -> OFF
    }
}

void checkModeDevice(){
  if(RUNAWAY_ERR) {
    heaterValue = TRIACOFF;
    return;
  }
  //--- режим реле = 0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]; 4-импульс ---
  switch (settings.sp_structs[1].mode) {
    uint8_t val;
    case 0:
      heaterValue = UpdatePID(0);            // ПИД нагреватель
      // MYDEBUG_PRINT("ПИД нагреватель:"); MYDEBUG_PRINTLN(heaterValue);
      humidiValue = UpdatePID(1);            // ПИД увлажнитель
      // MYDEBUG_PRINT("ПИД увлажнитель:"); MYDEBUG_PRINTLN(humidiValue);
      break;
    case 1:
      val = RelayPos(0,2);
      switch (val){
          case ON:  heaterValue = TRIACON;  break;
          case OFF: heaterValue = TRIACOFF; break;
      }
      // MYDEBUG_PRINT("РЕЛЕ нагреватель:"); MYDEBUG_PRINTLN(heaterValue);
      humidiValue = UpdatePID(1);            // ПИД увлажнитель
      // MYDEBUG_PRINT("ПИД увлажнитель:"); MYDEBUG_PRINTLN(humidiValue);
      break;
    case 2:
      heaterValue = UpdatePID(0);            // ПИД нагреватель
      // MYDEBUG_PRINT("ПИД нагреватель:"); MYDEBUG_PRINTLN(heaterValue);
      val = RelayPos(1,3);
      switch (val){
          case ON:  humidiValue = TRIACON;  break;
          case OFF: humidiValue = TRIACOFF; break;
      }
      // MYDEBUG_PRINT("РЕЛЕ увлажнитель:"); MYDEBUG_PRINTLN(humidiValue);
      break;
    case 3:
      val = RelayPos(0,2);
      switch (val){
          case ON:  heaterValue = TRIACON;  break;
          case OFF: heaterValue = TRIACOFF; break;
      }
      // MYDEBUG_PRINT("РЕЛЕ нагреватель:"); MYDEBUG_PRINTLN(heaterValue);
      val = RelayPos(1,3);
      switch (val){
          case ON:  humidiValue = TRIACON;  break;
          case OFF: humidiValue = TRIACOFF; break;
      }
      // MYDEBUG_PRINT("РЕЛЕ увлажнитель:"); MYDEBUG_PRINTLN(humidiValue);
      break;
    case 4:
      heaterValue = UpdatePID(0);           // ПИД нагреватель
      // MYDEBUG_PRINT("ПИД нагреватель:"); MYDEBUG_PRINTLN(heaterValue);
      OutPulse();                           // импульсное управление увлажнителем
      if (pvPeriod) --pvPeriod;
      else {
        pvPeriod = settings.sp_structs[1].pulse;  // начало нового периода
        if(pvPulse) humidiValue = TRIACON;        // включить канал 2 (импульсный режим)
      };
      // MYDEBUG_PRINT("ИМПУЛЬС увлажнитель pvPulse:"); MYDEBUG_PRINTLN(pvPulse);
      break;
    // default: MYDEBUG_PRINTLN("НЕТ нагреватель НЕТ увлажнитель"); break;
  }
}

uint8_t checkSetpoint(void){
  uint8_t err = 0;
  //--------- Загрузка конфигурации --------------------------------------------
  if(LittleFS.exists("/setpoint.json")){
      if(!loadSetpoint()){
        MYDEBUG_PRINTLN("Конфігурація не завантажена!");
        err = 1 ;
        saveSetpoint();  // значения по умолчанию
      }
  } else {
      saveSetpoint();  // значения по умолчанию
      MYDEBUG_PRINTLN("Конфігурація за замовчуванням!");
      err = 2 ;
  }
  MYDEBUG_PRINTLN("\n>> Итоговые значения после загрузки из FS:");
  #ifdef DEBUG
    printConfig();
  #endif
  return err;
}

uint8_t checkConfig(void){
  uint8_t err = 0;
  if(LittleFS.exists("/config.json")){
    //file exists, reading and loading
    MYDEBUG_PRINTLN("reading config file");
    File configFile = LittleFS.open("/config.json", "r");
    if(configFile){
      MYDEBUG_PRINTLN("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      JsonDocument json;
      auto deserializeError = deserializeJson(json, buf.get());
      serializeJson(json, Serial);
      if( ! deserializeError ){
        MYDEBUG_PRINTLN("\nparsed json");
        strcpy(botToken, json["botToken"]);
        strcpy(chatID, json["chatID"]);
      } else {
        MYDEBUG_PRINTLN("failed to load json config");
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
    MYDEBUG_PRINTLN("--------------------");
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
        DEBUG_PRINTF("  special: %d\n", settings.sp_structs[i].special);
    }
    MYDEBUG_PRINTLN("--------------------");
}
#endif

//----------- Функция сохранения конфигурации в JSON файл ----------------
void saveSetpoint() {
    MYDEBUG_PRINTLN("Сохранение конфигурации...");

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
        obj["special"] = settings.sp_structs[i].special;
    }

    // Открываем файл для записи
    File configFile = LittleFS.open("/setpoint.json", "w");
    if (!configFile) {
        MYDEBUG_PRINTLN("Не удалось открыть файл для записи");
        return;
    }

    // Сериализуем JSON в файл
    if (serializeJson(doc, configFile) == 0) {
        MYDEBUG_PRINTLN("Ошибка записи в файл");
    } else {
        MYDEBUG_PRINTLN("Конфигурация успешно сохранена.");
    }
    
    configFile.close();
}

//------------ Функция загрузки конфигурации из JSON файла -------------
bool loadSetpoint() {
    MYDEBUG_PRINTLN("Загрузка конфигурации...");

    // Открываем файл для чтения
    File configFile = LittleFS.open("/setpoint.json", "r");
    if (!configFile) {
        MYDEBUG_PRINTLN("Не удалось открыть файл для чтения. Используются значения по умолчанию.");
        return false;
    }

    // Создаем JSON документ для десериализации
    JsonDocument doc;

    // Десериализуем JSON из файла
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        MYDEBUG_PRINT("Ошибка десериализации JSON: ");
        MYDEBUG_PRINTLN(error.c_str());
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
            settings.sp_structs[i].special = obj["special"];
            i++;
        }
    }
    MYDEBUG_PRINTLN("Конфигурация успешно загружена.");
    return true;
}

#ifdef DEBUG
// Вспомогательная функция для вывода адреса датчика
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) MYDEBUG_PRINT("0");
    MYDEBUG_PRINT(deviceAddress[i], HEX);
    if (i < 7) MYDEBUG_PRINT(":");
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
errors = 0x01   // ОШИБКА ДАТЧИКА 0  SENSOR_ERROR_VAL; SENSOR_FROZEN_VAL [E01]
errors = 0x02   // ОШИБКА ДАТЧИКА 1  SENSOR_ERROR_VAL; SENSOR_FROZEN_VAL [E02]
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

// Вспомогательная функция для печати байта в двоичном формате
void printBinary(byte inByte) {
  for (int b = 5; b >= 0; b--) {
    switch (b) {
    case 5: MYDEBUG_PRINT("E3="); break;
    case 4: MYDEBUG_PRINT("E2="); break;
    case 3: MYDEBUG_PRINT("E1="); break;
    case 0: MYDEBUG_PRINT("TU="); break;
    }
    MYDEBUG_PRINT(bitRead(inByte, b)? "OF" : "ON");
    MYDEBUG_PRINTLN();
  }
}

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
  settings.sp_structs[0].special = SPECIAL0;

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
  settings.sp_structs[1].special = SPECIAL1;

  for (uint8_t i = 0; i < 8; i++) { data[i] = TOP;}
  module.setDisplay(data, 8); // Вывод на дисплей "--- --- --"
  beeperOn(50);
  delay(500);
  for (uint8_t i = 0; i < 8; i++) { data[i] = DEF;}
  module.setDisplay(data, 8); // Вывод на дисплей "--- --- --"
  beeperOn(50);
  delay(500);
  for (uint8_t i = 0; i < 8; i++) { data[i] = BOT;}
  module.setDisplay(data, 8); // Вывод на дисплей "--- --- --"
  saveSetpoint();
  beeperOn(100);
  delay(3000);
}

void newSecond(){
  errorsFlag.value = 0; 
          
  sensorCheck();        // Опрос датчиков должен быть всегда
  
  #ifdef DEBUG  
    // В режиме отладки можно оставить симуляцию, если датчики не подключены
    if(detectedSensor == UNKNOWN){
      dpv0 += pctHeater/50 + pid[0].iPart/10;
      ds[0].pvT = dpv0;
      dpv1 += pctHimidifier/50 + pid[1].iPart/10; 
      ds[1].pvT = dpv1;
    }
  #endif
    //--------------------------------- НАГРЕВАТЕЛЬ и УВЛАЖНИТЕЛЬ ----------------------------------------------------
      checkModeDevice();
      
      if(settings.sp_structs[0].mode == 1 && ds[0].deviation == 0) humidiValue = TRIACOFF; // задержка регулирования по 2 каналу до прогрева инкубатора
      //-----
      // DEBUG_SPRINTF(displStr,"Err=%i; pP0=%6.1f; out=%7.2f Heater=%u; iP0=%6.4f; Hum=%u; OUT=0x%02x; Pulse=%u; Timer=%u; Period=%u; Aera=%u; Vent=%u; Flap=%u",
      //   ds[0].pvErr,pid[0].pPart,pid[0].output,heaterValue,pid[0].iPart,humidiValue,portOut.value,pvPulse,pvTimer,pvPeriod,pvAeration,pvVenting,pvFlap);
      // MYDEBUG_PRINTLN(displStr);
      //-----
      
    //---------------------------- КАНАЛ ВСПОМОГАТЕЛЬНОГО НАГРЕВАТЕЛЯ -------------------------------------------------
      if(ERROR1 == 0){
        if(ds[0].pvErr >= settings.sp_structs[0].auxiliary) EXTRA2 = PCF_ON;        // включить вспомогательны нагреватель
        else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) EXTRA2 = PCF_OFF; // отключить вспомогательны нагреватель
      } else EXTRA2 = PCF_OFF;                                                      // отключить вспомогательны нагреватель
      // MYDEBUG_PRINT("ВСПОМОГАТЕЛЬНЫЙ НАГРЕВАТЕЛь:"); MYDEBUG_PRINTLN(EXTRA2 ? "OFF" : "ON");
    //--------------------------------------- ПРОВЕТРИВАНИЕ -----------------------------------------------------------
      if(AERATION){     // Идет ПРОВЕТРИВАНИЕ !
        EXTRA1 = PCF_ON; pvFlap = 100; beeperOn(10);
        if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; AERATION =0; EXTRA1 = PCF_OFF;}
        // MYDEBUG_PRINT("ПРОВЕТРИВАНИЕ:"); MYDEBUG_PRINTLN(EXTRA1 ? "OFF" : "ON");
      } else {
    //------------------------------------ ОХЛАЖДЕНИЕ  ОСУШЕНИЕ --------------------------------------------------------
        uint8_t val = RelayNeg(0,settings.sp_structs[0].coolOn,settings.sp_structs[0].coolOff);
        if(val == OFF && (ds[0].pvErr <= settings.sp_structs[0].alarm)) 
                val = RelayNeg(1,settings.sp_structs[1].coolOn,settings.sp_structs[1].coolOff); // если холодно то не открываем заслонку.
        if(val == ON){EXTRA1 = PCF_ON; pvFlap = 100;} else if(val == OFF){EXTRA1 = PCF_OFF; pvFlap = settings.sp_structs[0].state;}
        // MYDEBUG_PRINT("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ:"); MYDEBUG_PRINTLN(EXTRA1 ? "OFF" : "ON");
      }
    //-------------------------------------- ПОЛОЖЕНИЕ ЗАСЛОНКИ --------------------------------------------------------
      // setflap();                            // задание положения заслонки 
      // MYDEBUG_PRINT("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ:"); MYDEBUG_PRINTLN(EXTRA1);
    //-------------------------------- ПОВОРОТ ЛОТКОВ асимметричный режим ----------------------------------------------
      if(settings.sp_structs[1].timer && TURN){// только при sp[1].timer>0 -> асимметричный режим
        TURNSECOND = ON;
        if(--pvTimer==0){
          MYDEBUG_PRINTLN("асимметричный режим: TURN = OFF;");
          pvTimer = settings.sp_structs[0].timer; 
          TURN = PCF_OFF; TURNSECOND = OFF;
        }
      } else {
        TURNSECOND = OFF;
      }
    //-------------------------------------------- АВАРИЯ -------------------------------------------------------------
    if(numSetup == 0){
      uint8_t res = alarm();
      switch (settings.sp_structs[0].extendMode){
      case 0: EXTRA3 = !res; break;                  // [0]-0-СИРЕНА;
      case 1:
          uint8_t val = RelayNeg(0,settings.sp_structs[0].alarm,settings.sp_structs[0].spT); // 1-АВАРИЙНОЕ ВЫКЛЮЧЕНИЕ.
          if(val == ON) EXTRA3 = PCF_ON;                // включить
          else if(val == OFF) EXTRA3 = PCF_OFF;         // отключить
        break;
      }
    }
    pctHeater = constrain(heaterValue, 0, 255);
    pctHeater = map(pctHeater,0,255,0,100);
    
    pctHimidifier = constrain(humidiValue, 0, 255);
    pctHimidifier = map(pctHimidifier,0,255,0,100);
    /*
    DEBUG_SPRINTF(displStr,"T0=%5.1f; T1=%5.1f; Err=%i; pP0=%6.1f; iP0=%6.4f; Heater=%u; PW=%u%%; Err=%i; pP1=%6.1f; iP1=%6.4f; Himid=%u; HU=%u%%; Tim=%u;  ERR=0x%02x; time: %u;",
      (float)ds[0].pvT/10,(float)ds[1].pvT/10,ds[0].pvErr,pid[0].pPart,pid[0].iPart,heaterValue,pctHeater,ds[1].pvErr,pid[1].pPart,pid[1].iPart,humidiValue,pctHimidifier,pvTimer,errorsFlag.value,countSeconds);
    MYDEBUG_PRINTLN(displStr);
    // printBinary(portOut.value);
    MYDEBUG_PRINTLN("==============================================================================");
    MYDEBUG_PRINTLN(); */
}

void newMinute(){
  //---------------------------- ЗАЩИТА ОТ РАЗНОСА -------------------------
  if(pctHeater == 100){
    // Если текущая температура не выше той, что была минуту назад
    if(ds[0].pvT <= lastMinuteT){
       if(++stagnationTimer > 5){ // 5 минут нет роста при 100% мощности
          stagnationTimer = 5;
          // Если мы все еще далеко от уставки (ошибка больше аварийного порога)
          if(ds[0].pvErr > settings.sp_structs[0].alarm) {
             RUNAWAY_ERR = 1;
             heaterValue = TRIACOFF; // Принудительно выключаем нагрев для безопасности
             MYDEBUG_PRINTLN("!!! WARNING: Runaway protection triggered (stagnation) !!!");
          }
       }
    } else {
       stagnationTimer = 0; // Есть рост - сбрасываем счетчик
       RUNAWAY_ERR = 0;
    }
  } else {
    stagnationTimer = 0;
    RUNAWAY_ERR = 0;
  }
  lastMinuteT = ds[0].pvT; // Запоминаем температуру для следующей минуты
  //------------------------------------------------------------------------
  countSeconds = 0;
    if(++countMinutes > 59){
      countMinutes = 0;
      if(++countHours > 23){
        countHours = 0;
        if(++countDays > 30) countDays = 1;
      } 
    }
  // }
  // DEBUG_PRINTF("=== НОВАЯ МИНУТА: %02u:%02u:00\n",countHours,countMinutes);
  // DEBUG_PRINTF("Free heap size: %d\n", ESP.getFreeHeap());  // Проверка доступной памяти
  if(disableBeep) disableBeep--;
  //---------------------------- ПОВОРОТ ЛОТКОВ ----------------------------
  // if(settings.sp_structs[0].timer) rotate_trays();
  //---------------------------- ПРОВЕТРИВАНИЕ !! --------------------------
  
  //------------------------ СОХРАНЕНИЕ ТЕМПЕРАТУРЫ ------------------------
  if((countMinutes % 5) == 0){
    now = rtc.now();       // текущее время из DS3231 в формате Unix, сконвертированное для нашего часового пояса
    DEBUG_PRINTF("=== НОВАЯ ЗАПИСЬ:%02u.%02u day  %02u:%02u:00\n",now.month(),countDays,countHours,countMinutes);
    DEBUG_PRINTF("=== ----- ЗАПИСЬ:%02u.%02u day  %02u:%02u:%02u\n",now.month(),now.day(),now.hour(),now.minute(),now.second());
    int period_of_day = (countHours * 60 + countMinutes) / 5; // Вычисляем номер 5-минутного периода в сутках (от 0 до 287)
    int address = period_of_day * sizeof(int16_t) * 2;        // Вычисляем адрес на основе этого периода
    // Записываем в EEPROM
    if (countDays != lastDayProcessed && countDays <= 30) {
      saveDailyDataToFile(lastDayProcessed);
      lastDayProcessed = countDays;
    } 
    eepromWriteInt16(address, ds[0].pvT);
    eepromWriteInt16(address + sizeof(int16_t), ds[1].pvT);
    DEBUG_PRINTF("PerOfDay=%03u; Addr0: 0x%04x; t0=%6.1f; Addr1: 0x%04x; t1=%6.1f;",period_of_day,address,(float)ds[0].pvT/10,address + sizeof(int16_t),(float)ds[1].pvT/10);
  }
}
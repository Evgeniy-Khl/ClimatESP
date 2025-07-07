#include "main.h"

#define UNALTERED   2 // неизменный

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
  if(TURN){
    if(--pvTimer == 0){pvTimer = settings.sp_structs[0].timer; TURN = OFF;}
  } else {
    if(--pvTimer == 0){
        if(settings.sp_structs[1].timer){pvTimer = settings.sp_structs[1].timer; TURN = ON;}
        else {pvTimer = settings.sp_structs[0].timer; TURN = ON;}
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

//-------- Функция для печати текущих значений структуры в Serial порт --------
#ifdef DEBUG
void printConfig() {
    DEBUG_PRINTLN("--------------------");
    for (int i = 0; i < 2; i++) {
        Serial.printf("Элемент settings.sp_structs[%d]:\n", i);
        Serial.printf("  spT: %d\n", settings.sp_structs[i].spT);
        Serial.printf("  spRH: %d\n", settings.sp_structs[i].spRH);
        Serial.printf("  alarm: %d\n", settings.sp_structs[i].alarm);
        Serial.printf("  coolOn: %d\n", settings.sp_structs[i].coolOn);
        Serial.printf("  coolOff: %d\n", settings.sp_structs[i].coolOff);
        Serial.printf("  timer: %d\n", settings.sp_structs[i].timer);
        Serial.printf("  aeration: %d\n", settings.sp_structs[i].aeration);
        Serial.printf("  auxiliary: %d\n", settings.sp_structs[i].auxiliary);
        Serial.printf("  flapLimit: %d\n", settings.sp_structs[i].flapLimit);
        Serial.printf("  state: %d\n", settings.sp_structs[i].state);
        Serial.printf("  pulse: %d\n", settings.sp_structs[i].pulse);
        Serial.printf("  mode: %d\n", settings.sp_structs[i].mode);
        Serial.printf("  extendMode: %d\n", settings.sp_structs[i].extendMode);
        Serial.printf("  Kp: %d\n", settings.sp_structs[i].Kp);
        Serial.printf("  Ki: %d\n", settings.sp_structs[i].Ki);
    }
    DEBUG_PRINTLN("--------------------");
}
#endif

//----------- Функция сохранения конфигурации в JSON файл ----------------
void saveConfig() {
    DEBUG_PRINTLN("Сохранение конфигурации...");

    // Создаем JSON документ. Размер 512 байт более чем достаточен.
    StaticJsonDocument<1024> doc;

    // Создаем корневой JSON массив
    JsonArray jsonArray = doc.to<JsonArray>();

    // Проходим по массиву структур и добавляем данные в JSON
    for (int i = 0; i < 2; i++) {
        JsonObject obj = jsonArray.createNestedObject();
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
    StaticJsonDocument<1024> doc;

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
    if(disableBeep==0) {beepOn = lower; cn = ON;};// длительность звукового сигнала и включить канал 4 (6 А)
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

  saveConfig();
}
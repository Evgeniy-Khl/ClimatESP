
#include "main.h"
#include "my_settings.h"
char displStr[200];

// AsyncWebServer server(80);      // Create AsyncWebServer object on port 80
ESP8266WebServer server(80);
WiFiClientSecure client;
MyTelegramBot bot(botToken, client);

PIDController pid[2];
SoftwarePWMBit heaterPwm(&portOut.value, 0); 
SoftwarePWMBit humidiPwm(&portOut.value, 1);

RTC_DS3231 rtc;                     // Создаем объект RTC для DS3231

OneWire oneWire(ONE_WIRE_BUS_PIN);  // Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
DallasTemperature sensors(&oneWire);// Передаем ссылку на объект oneWire в конструктор DallasTemperature

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

#ifdef LED_DISPLAY
  TM1638 module(13, 14, 12);    // Создаем объект module для TM1638
  void ledDisplKeypad(long now);
  void ledSet(void);
#else

#endif
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);       // Инициализация последовательного порта для отладки
  #endif
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");   // get UTC time via NTP
    client.setTrustAnchors(&cert);      // Add root certificate for api.telegram.org
  #endif
  //------------------------- read configuration from FS json ----------------------------------------
  Serial.println("mounting FS...");
  bool lFS = LittleFS.begin();
    if(lFS) {
      Serial.println("mounted file system");
      if(LittleFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = LittleFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);

          JsonDocument json;
          auto deserializeError = deserializeJson(json, buf.get());
          serializeJson(json, Serial);
          if ( ! deserializeError ) {
            Serial.println("\nparsed json");
            strcpy(botToken, json["botToken"]);
            strcpy(chatID, json["chatID"]);
          } else {
            Serial.println("failed to load json config");
          }
          configFile.close();
        }
      }
    } else {
      Serial.println("failed to mount FS");
    }
  //---------------------------------------------------------------------------------------clean LittleFS, for testing-----------------
// **Здесь вы можете разместить LittleFS.format();  но ОЧЕНЬ ВАЖНО ПОНИМАТЬ КОГДА ЭТО ДЕЛАТЬ!**
// Например, вы можете отформатировать файловую систему только при первом запуске или при определенном условии.
// **ВНИМАНИЕ: Раскомментирование следующей строки приведет к форматированию LittleFS при каждом запуске!**
// Проверка и форматирование, если необходимо
    // if (LittleFS.format()) {
    //   Serial.println("LittleFS formatted successfully");
    // } else {
    //   Serial.println("Failed to format LittleFS");
    // }
//-------------------------------------------------------
    // Получение информации о файловой системе
    FSInfo fs_info;
    LittleFS.info(fs_info);

    Serial.printf("Total space: %u bytes\n", fs_info.totalBytes);
    Serial.printf("Used space: %u bytes\n", fs_info.usedBytes);
    Serial.printf("Free space: %u bytes\n", fs_info.totalBytes - fs_info.usedBytes);
    //end read
    //---------------------------- инициализация WiFiManager -----------------------------------
    initWiFiManag();
    //------------------------------------------------------------------------------------------
  #ifdef LED_DISPLAY
    pinMode(BEEP_PIN, OUTPUT);    // Настраиваем пин бипера как выход
    initLedConfig(lFS);
  #else

  #endif
  pvTimer = settings.sp_structs[0].timer;                  // инициализация времени выключенного состояния таймера
  pvAeration = settings.sp_structs[0].aeration;            // инициализация ПАУЗы ПРОВЕТРИВАНИЯ (минут)
  heaterPwm.write(heaterValue);
  humidiPwm.write(humidiValue);
  portOut.value = 0;
  delay(3000);
}

void loop() {
  //--------------------------- УПРАВЛЕНИЕ СИМИСТОРОМ ---------------------------------
  bool hasChanged = false;
  long now = millis();
  hasChanged |= heaterPwm.update();
  hasChanged |= humidiPwm.update();
  if(hasChanged){
    // writePCF8574(portOut.value);
    // sprintf(displStr,"*** NOW = %lu; VAL = %u; **",now,portOut.value);
    // DEBUG_PRINTLN(displStr);
  }
  //============================= ПРОВЕРКА по таймеру =================================
  #ifdef LED_DISPLAY
    ledDisplKeypad(now);
  #endif
  //================================ НОВАЯ СЕКУНДА =================================
    if(halfSecond & 2){
        errorsFlag.value = 0;
      #ifndef DEBUG  
        temperature_check();
      
        if (HIH5030){
          uint16_t adc=1024;
          pvVadcRH = adc;//lowPassF2(adc);           // относительная влажность в Vadc ??????????????????????????????????????????
          if (pvVadcRH>80) pvRH = valDcToRH(pvVadcRH); // относительная влажность в %
          else pvRH = 1990;
        } else {
          uint8_t valTable = tableRH(ds[0].pvT, ds[1].pvT);               // если отсутствует HIH4000 то ...
          if(valTable>100) pvRH = 100; else pvRH = valTable;
        }
      #else
        //-----температура воздуха------
        heaterValue = UpdatePID(0);            // ПИД нагреватель
        // humidiValue = UpdatePID(1);            // ПИД увлажнитель
        //-----
        // sprintf(displStr,"Err=%i; pP0=%6.1f; out=%7.2f Heater=%u; iP0=%6.4f; Hum=%u; OUT=0x%02x; Pulse=%u; Timer=%u; Period=%u; Aera=%u; Vent=%u; Flap=%u",
        //   ds[0].pvErr,pid[0].pPart,pid[0].output,heaterValue,pid[0].iPart,humidiValue,portOut.value,pvPulse,pvTimer,pvPeriod,pvAeration,pvVenting,pvFlap);
        // DEBUG_PRINTLN(displStr);
        //-----
        heaterPwm.write(heaterValue);
        humidiPwm.write(humidiValue);
        dpv0 = pid[0].pPart/250 + pid[0].iPart*8;
        ds[0].pvT += dpv0;
        dpv1 = pid[1].pPart/250 + pid[1].iPart*8;
        ds[1].pvT += dpv1;
        //------
        // DEBUG_PRINTLN();
        // sprintf(displStr,"=== Sek = %u; ResD = %u; DspN = %u; SetN = %u ===",halfSecond/2,resetDispl,displNum,numSetup);
        // DEBUG_PRINTLN(displStr);
        
        // sprintf(displStr,"pP0 = %g; iP0 = %g; out = %g;",pid[0].pPart,pid[0].iPart,pid[0].output);
        // DEBUG_PRINTLN(displStr);
        
        
        // Serial.flush();
        //------
      #endif
        if(!COOLING){  //-------------- нормальная работа -------------------------
          //--- режим реле = 0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]; 4-импульс ---
          switch (settings.sp_structs[1].mode) {
              uint8_t val;
              case 0:
                heaterValue = UpdatePID(0);            // ПИД нагреватель
                DEBUG_PRINT("ПИД нагреватель:"); DEBUG_PRINTLN(heaterValue);
                humidiValue = UpdatePID(1);            // ПИД увлажнитель
                DEBUG_PRINT("ПИД увлажнитель:"); DEBUG_PRINTLN(humidiValue);
                break;
              case 1:
                val = RelayPos(0,2);
                switch (val){
                    case ON: heaterValue = TRIACON; break;
                    case OFF: heaterValue = OFF;    break;
                }
                DEBUG_PRINT("РЕЛЕ нагреватель:"); DEBUG_PRINTLN(heaterValue);
                humidiValue = UpdatePID(1);            // ПИД увлажнитель
                DEBUG_PRINT("ПИД увлажнитель:"); DEBUG_PRINTLN(humidiValue);
                break;
              case 2:
                heaterValue = UpdatePID(0);            // ПИД нагреватель
                DEBUG_PRINT("ПИД нагреватель:"); DEBUG_PRINTLN(heaterValue);
                val = RelayPos(1,3);
                switch (val){
                    case ON: humidiValue = TRIACON; break;
                    case OFF: humidiValue = OFF;    break;
                }
                DEBUG_PRINT("РЕЛЕ увлажнитель:"); DEBUG_PRINTLN(humidiValue);
                break;
              case 3:
                val = RelayPos(0,2);
                switch (val){
                    case ON: heaterValue = TRIACON; break;
                    case OFF: heaterValue = OFF;    break;
                }
                DEBUG_PRINT("РЕЛЕ нагреватель:"); DEBUG_PRINTLN(heaterValue);
                val = RelayPos(1,3);
                switch (val){
                    case ON: humidiValue = TRIACON; break;
                    case OFF: humidiValue = OFF;    break;
                }
                DEBUG_PRINT("РЕЛЕ увлажнитель:"); DEBUG_PRINTLN(humidiValue);
                break;
              case 4:
                heaterValue = UpdatePID(0);           // ПИД нагреватель
                DEBUG_PRINT("ПИД нагреватель:"); DEBUG_PRINTLN(heaterValue);
                OutPulse();                           // импульсное управление увлажнителем
                if (pvPeriod) --pvPeriod;
                else {
                  pvPeriod = settings.sp_structs[1].pulse;  // начало нового периода
                  if(pvPulse) humidiValue = TRIACON;        // включить канал 2 (импульсный режим)
                };
                DEBUG_PRINT("ИМПУЛЬС увлажнитель pvPulse:"); DEBUG_PRINTLN(pvPulse);
                break;
              default: DEBUG_PRINTLN("НЕТ нагреватель НЕТ увлажнитель");
                break;
          }
        } else {heaterValue = 0; displPower = 0;}      // иначе идет ОХЛАЖДЕНИЕ!

        if(settings.sp_structs[0].mode == 1 && !REACHED0) humidiValue=OFF; // задержка регулирования по 2 каналу до прогрева инкубатора

        // heaterPwm.write(heaterValue);
        // humidiPwm.write(humidiValue);

        if(!COOLING){  //-------------- нормальная работа -------------------------
          //------ КАНАЛ ВСПОМОГАТЕЛЬНОГО НАГРЕВАТЕЛЯ -------------------------------------------------
          if(ERROR1 == 0){
            if(ds[0].pvErr >= settings.sp_structs[0].auxiliary) EXTRA2 = ON;        // включить вспомогательны нагреватель
            else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) EXTRA2 = OFF; // отключить вспомогательны нагреватель
          } else EXTRA2 = OFF;                                                      // отключить вспомогательны нагреватель
          DEBUG_PRINT("ВСПОМОГАТЕЛЬНЫЙ НАГРЕВАТЕЛь:"); DEBUG_PRINTLN(EXTRA2);
        //------------------------- ПРОВЕТРИВАНИЕ -----------------------------
          if(AERATION){     // Идет ПРОВЕТРИВАНИЕ !
            EXTRA1 = ON; pvFlap = 100; beeperOn(10);
            if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; AERATION =0; EXTRA1 = OFF;}
            DEBUG_PRINT("ПРОВЕТРИВАНИЕ:"); DEBUG_PRINTLN(EXTRA1);
          } else {
          //------ ОХЛАЖДЕНИЕ  ОСУШЕНИЕ ---------------------------------------------------------------
            uint8_t val = RelayNeg(0,settings.sp_structs[0].coolOn,settings.sp_structs[0].coolOff);
            if(val == OFF && (ds[0].pvErr <= settings.sp_structs[0].alarm)) 
                    val = RelayNeg(1,settings.sp_structs[1].coolOn,settings.sp_structs[1].coolOff); // если холодно то не открываем заслонку.
            if(val == ON){EXTRA1 = ON; pvFlap = 100;} else if(val == OFF){EXTRA1 = OFF; pvFlap = settings.sp_structs[0].state;}
            DEBUG_PRINT("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ:"); DEBUG_PRINTLN(EXTRA1);
          }
        }
      //------------------------- ПОЛОЖЕНИЕ ЗАСЛОНКИ ---------------------------
        // setflap();                            // задание положения заслонки 
        // DEBUG_PRINT("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ:"); DEBUG_PRINTLN(EXTRA1);

      //---------------------------- ПОВОРОТ ЛОТКОВ ----------------------------
      if(settings.sp_structs[1].timer && TURN){// только при sp[1].timer>0 -> асимметричный режим
        if(--pvTimer==0){
          pvTimer = settings.sp_structs[0].timer; 
          TURN = OFF;
        }
      }
      //--------------------------------- АВАРИЯ ---------------------------------
      if(numSetup == 0){
        uint8_t res = alarm();
        switch (settings.sp_structs[0].extendMode){
        case 0: EXTRA3 = res; break;                  // [0]-0-СИРЕНА;
        case 1:
            uint8_t val = RelayNeg(0,settings.sp_structs[0].alarm,settings.sp_structs[0].spT); // 1-АВАРИЙНОЕ ВЫКЛЮЧЕНИЕ.
            if(val == ON) EXTRA3 = ON;                // включить
            else if(val == OFF) EXTRA3 = OFF;         // отключить
          break;
        }
      }
    }//============================== КОНЕЦ СЕКУНДЫ =================================
    // DateTime now = rtc.now();

    
    //==================== НОВАЯ МИНУТА =======================================
    if(halfSecond == 0){
      // DEBUG_PRINTLN("=== НОВАЯ МИНУТА ===");
      if(disableBeep) disableBeep--;
      //---------------------------- ПОВОРОТ ЛОТКОВ ----------------------------
      if(settings.sp_structs[0].timer) rotate_trays();
      //---------------------------- ПРОВЕТРИВАНИЕ !! --------------------------
      if(!AERATION && !COOLING && settings.sp_structs[1].aeration){
        if(--pvAeration == 0){
          pvVenting = settings.sp_structs[1].aeration; AERATION = 1; EXTRA1 = ON;
          //  if((relayMode & 4) && checkDry==0) {pwTriac1=maxRun; CN2 = CN2ON;}// принудительный впрыск воды!!!
        }
      } else if(COOLING){
        EXTRA1 = ON; pvFlap = 100; beeperOn(50);
        if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; COOLING = 0;}
      }
    }//==================== КОНЕЦ МИНУТЫ  ===================================

    sprintf(displStr,"T0 = %5.1f; T1 = %5.1f; OUT=0x%02x; ERR=0x%02x;",(float)ds[0].pvT/10,(float)ds[1].pvT/10,portOut.value,errorsFlag.value);
    DEBUG_PRINTLN(displStr);
  #ifdef LED_DISPLAY
    ledSet();
  #endif
}
//----------------------------------- loop() ----------------------------------------------------------------------

// Функция для записи байта на PCF8574
byte writePCF8574(byte data) {
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(data);
  byte error = Wire.endTransmission();
  if (error == 0) {
    //DEBUG_PRINT("Data written: 0b");
    //printBinary(data);
    //DEBUG_PRINTLN();
  } else {
    DEBUG_PRINT("Error writing to PCF8574. Error code: ");
    DEBUG_PRINTLN(error);
  }
  return error;
}

// Функция для чтения байта с PCF8574
byte readPCF8574() {
  Wire.requestFrom(PCF8574_ADDRESS, 1); // Запросить 1 байт данных
  if (Wire.available()) {
    return Wire.read();
  } else {
    DEBUG_PRINTLN("Error reading from PCF8574: No data available.");
    return 0xFF; // Возвращаем 0xFF в случае ошибки (можно выбрать другое значение)
  }
}

// Вспомогательная функция для печати байта в двоичном формате
void printBinary(byte inByte) {
  for (int b = 7; b >= 0; b--) {
    DEBUG_PRINT(bitRead(inByte, b));
  }
}


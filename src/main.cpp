#include "main.h"
#include "my_settings.h"
char displStr[200];

ESP8266WebServer server(80);
WiFiClientSecure client;
MyTelegramBot bot(botToken, client);

PIDController pid[2];
SoftwarePWMBit heaterPwm(&portOut.value, 1); 
SoftwarePWMBit humidiPwm(&portOut.value, 2);

RTC_DS3231 rtc;                     // Создаем объект RTC для DS3231

DHT dht(ONE_WIRE_BUS_PIN, DHT22);
OneWire oneWire(ONE_WIRE_BUS_PIN);  // Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
DallasTemperature sensors(&oneWire);// Передаем ссылку на объект oneWire в конструктор DallasTemperature
DeviceAddress sensorAddresses[MAX_DEVICE];  // Массив для хранения уникальных адресов датчиков

byte writePCF8574(byte data);

TM1638 module(13, 14, 12);    // Создаем объект module для TM1638
void ledSet(void);

void setup(){
  #ifdef DEBUG
    Serial.begin(115200);               // Инициализация последовательного порта для отладки
  #endif

  Wire.begin();                         // Инициализация I2C (SDA, SCL по умолчанию для ESP8266 - GPIO4, GPIO5)
  uint8_t temp = writePCF8574(0xFF);    // Установить все пины в LOW (если они используются как выходы)

  for (uint8_t i = 0; i < 8; i++) { data[i] = OO;}
  if(temp) data[7] = NUMBER_FONT[14];   //"ooo ooo oE"
  //----------------------------------- MOUNTING FS ----------------------------------------
  MYDEBUG_PRINTLN("mounting FS...");
  bool lFS = LittleFS.begin();
  if(lFS) {
    MYDEBUG_PRINTLN("mounted file system");
    //--------------------------------- clean LittleFS, for testing -----------------------
    // **Здесь вы можете разместить LittleFS.format();  но ОЧЕНЬ ВАЖНО ПОНИМАТЬ КОГДА ЭТО ДЕЛАТЬ!**
    // Например, вы можете отформатировать файловую систему только при первом запуске или при определенном условии.
    // **ВНИМАНИЕ: Раскомментирование следующей строки приведет к форматированию LittleFS при каждом запуске!**
    // Проверка и форматирование, если необходимо
    // if (LittleFS.format()) {
    //   Serial.println("LittleFS formatted successfully");
    // } else {
    //   Serial.println("Failed to format LittleFS");
    // }
    //--------------------- checkSetpoint ----------------------------------
    dataLed[4] = checkSetpoint();
    dataLed[5] = checkConfig();
  } else {
    MYDEBUG_PRINTLN("failed to mount FS");
    data[6] = NUMBER_FONT[14];          //"ooo ooo Eo"
  }
  //---------------------------- инициализация WiFiManager -----------------------------------
  if(settings.sp_structs[0].special & 0x03) initWiFiManag();
  else MYDEBUG_PRINTLN("Запрет на подключение к WiFi! Продолжаем работу в оффлайн-режиме.");
  //------------------------------------------------------------------------------
  PID_Init(&pid[0], settings.sp_structs[0].Kp, settings.sp_structs[0].Ki);
  PID_Init(&pid[1], settings.sp_structs[1].Kp, settings.sp_structs[1].Ki);
  DEBUG_SPRINTF(displStr,"Пропорц.0= %g  Ітеграл.0= %g", pid[0].Kp,pid[0].Ki);
  MYDEBUG_PRINTLN(displStr);
  DEBUG_SPRINTF(displStr,"Пропорц.1= %g  Ітеграл.1= %g", pid[1].Kp,pid[1].Ki);
  MYDEBUG_PRINTLN(displStr);
  //---------- Инициализация DS3231 ----------------------------------------
  if(rtc.begin()) {RTCENABLE = 1; data[5] = NUMBER_FONT[12];}          //"ooo ooC oo"
  //------------------------------------------------------------------------------
  testProgs();              // тест
  //----------------------- определяем какой датчик подключен --------------------------------
  sensorType();
  switch (detectedSensor){
    case SENSOR_DS18B20: data[0] = NUMBER_FONT[numberOfDevices]; break;
    case SENSOR_DHT22:   data[1] = NUMBER_FONT[1]; break;
    case UNKNOWN: 
          data[0] = NUMBER_FONT[0];
          data[1] = NUMBER_FONT[0];
      break;
  }
  //------------------------------------------------------------------------------------------
  digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
  pinMode(BEEP_PIN, OUTPUT);    // Настраиваем пин бипера как выход только для LED
  displ_IP();
  pvTimer = settings.sp_structs[0].timer;                  // инициализация времени выключенного состояния таймера
  pvAeration = settings.sp_structs[0].aeration;            // инициализация ПАУЗы ПРОВЕТРИВАНИЯ (минут)
  heaterPwm.write(heaterValue);
  humidiPwm.write(humidiValue);
  portOut.value = 0xFF;
  module.setDisplay(data, 8);           // Вывод на дисплей "ooo ooo oo"
  delay(3000);
}

void loop(){
  //--------------------------- УПРАВЛЕНИЕ СИМИСТОРОМ ---------------------------------
  bool hasChanged = false;
  long now = millis();
  
  server.handleClient(); // Обработка входящих запросов
  //-------------------------------------------- 10 mSec. --------------------------------------
  if(now - counter10 > 10){
    counter10 = now;
    hasChanged |= heaterPwm.update();
    hasChanged |= humidiPwm.update();

    if(hasChanged) {
      writePCF8574(portOut.value);
      #ifdef LED_DISPLAY
      ledSet();
      #endif
    }

    if(beepOn) beepOn--; else digitalWrite(BEEP_PIN, HIGH);   // Выключаем бипер

    if(settings.sp_structs[0].mode == 4 && --pvPulse == 0){   // импульсный режим увлажнения
      humidiValue = TRIACOFF;
      writePCF8574(portOut.value);
      #ifdef LED_DISPLAY
      ledSet();
      #endif
    }

    #ifdef LED_DISPLAY
    keys = module.getButtons();
    if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}  // если не удерживается ни одна кнопка то сброс времени ожидания.
    #endif
  }
  #ifdef LED_DISPLAY
  //-------------------------------------------- КЛАВИАТУРА --------------------------------------
    if(now - counterWait > waitCheckKeyPad){
      counterWait = now;
      keys = module.getButtons();
      
      if(lastKey == keys && keys > 0){
        keyCount++;
        checkkey(keys);
        if(numSetup == 0) ledDispl(displNum);
        else display_setup();
        module.setDisplay(data, 8);
      } 
      else if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}
      else lastKey = keys;
    }
  #endif

  //============================= НОВАЯ ПОЛ-СЕКУНДА =================================
  if(now - counter1s > 500){
    counter1s = now; 
    if(++halfSecond > 119) halfSecond = 0;

    if(resetDispl) --resetDispl;
    else if(numSetup) saveset();  // сохраняем установки
    else displNum = 0;            // возврат к главному дисплею

  #ifdef LED_DISPLAY
    if(numSetup == 0) ledDispl(displNum); else display_setup();
    module.setDisplay(data, 8);
  #endif  
  //================================ НОВАЯ СЕКУНДА =================================
    if(halfSecond % 2){
        errorsFlag.value = 0; 
        if(++countSeconds > 59){
          countSeconds = 0;
          //==================== НОВАЯ МИНУТА =======================================
          MYDEBUG_PRINTLN("=== НОВАЯ МИНУТА ===");
          if(disableBeep) disableBeep--;
          //---------------------------- ПОВОРОТ ЛОТКОВ ----------------------------
          if(settings.sp_structs[0].timer) rotate_trays();
          //---------------------------- ПРОВЕТРИВАНИЕ !! --------------------------
          if(!AERATION && !COOLING && settings.sp_structs[1].aeration){
            if(--pvAeration == 0){
              pvVenting = settings.sp_structs[1].aeration; AERATION = 1; EXTRA1 = PCF_ON;
              //  if((relayMode & 4) && checkDry==0) {pwTriac1=maxRun; CN2 = CN2ON;}// принудительный впрыск воды!!!
            }
          } else if(COOLING){
            EXTRA1 = PCF_ON; pvFlap = 100; beeperOn(50);
            if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; COOLING = 0;}
          }
        } //==================== КОНЕЦ МИНУТЫ  ===================================
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
        // DEBUG_SPRINTF(displStr,"Err=%i; pP0=%6.1f; out=%7.2f Heater=%u; iP0=%6.4f; Hum=%u; OUT=0x%02x; Pulse=%u; Timer=%u; Period=%u; Aera=%u; Vent=%u; Flap=%u",
        //   ds[0].pvErr,pid[0].pPart,pid[0].output,heaterValue,pid[0].iPart,humidiValue,portOut.value,pvPulse,pvTimer,pvPeriod,pvAeration,pvVenting,pvFlap);
        // MYDEBUG_PRINTLN(displStr);
        //-----
        // heaterPwm.write(heaterValue);
        // humidiPwm.write(humidiValue);
        dpv0 = pid[0].pPart/250 + pid[0].iPart*8;
        ds[0].pvT += dpv0;
        dpv1 = pid[1].pPart/250 + pid[1].iPart*8;
        ds[1].pvT += dpv1;

        uint8_t valTable = tableRH(ds[0].pvT, ds[1].pvT);               // если отсутствует HIH4000 то ...
        if(valTable == 255) pvRH = valTable;
        else if(valTable > 100) pvRH = 100;
        else pvRH = valTable;
        //------
        // MYDEBUG_PRINTLN();
        // DEBUG_SPRINTF(displStr,"=== Sek = %u; ResD = %u; DspN = %u; SetN = %u ===",halfSecond/2,resetDispl,displNum,numSetup);
        // MYDEBUG_PRINTLN(displStr);
        
        // DEBUG_SPRINTF(displStr,"pP0 = %g; iP0 = %g; out = %g;",pid[0].pPart,pid[0].iPart,pid[0].output);
        // MYDEBUG_PRINTLN(displStr);
        
        
        // Serial.flush();
        //------
      #endif
        if(!COOLING){  //-------------- нормальная работа -------------------------
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
        } else {heaterValue = TRIACOFF; pctHeater = 0;}      // иначе идет ОХЛАЖДЕНИЕ!

        if(settings.sp_structs[0].mode == 1 && !REACHED0) humidiValue = TRIACOFF; // задержка регулирования по 2 каналу до прогрева инкубатора

        // heaterPwm.write(heaterValue);
        // humidiPwm.write(humidiValue);

        if(!COOLING){  //-------------- нормальная работа -------------------------
          //------ КАНАЛ ВСПОМОГАТЕЛЬНОГО НАГРЕВАТЕЛЯ -------------------------------------------------
          if(ERROR1 == 0){
            if(ds[0].pvErr >= settings.sp_structs[0].auxiliary) EXTRA2 = PCF_ON;        // включить вспомогательны нагреватель
            else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) EXTRA2 = PCF_OFF; // отключить вспомогательны нагреватель
          } else EXTRA2 = PCF_OFF;                                                      // отключить вспомогательны нагреватель
          // MYDEBUG_PRINT("ВСПОМОГАТЕЛЬНЫЙ НАГРЕВАТЕЛь:"); MYDEBUG_PRINTLN(EXTRA2 ? "OFF" : "ON");
        //------------------------- ПРОВЕТРИВАНИЕ -----------------------------
          if(AERATION){     // Идет ПРОВЕТРИВАНИЕ !
            EXTRA1 = PCF_ON; pvFlap = 100; beeperOn(10);
            if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; AERATION =0; EXTRA1 = PCF_OFF;}
            // MYDEBUG_PRINT("ПРОВЕТРИВАНИЕ:"); MYDEBUG_PRINTLN(EXTRA1 ? "OFF" : "ON");
          } else {
          //------ ОХЛАЖДЕНИЕ  ОСУШЕНИЕ ---------------------------------------------------------------
            uint8_t val = RelayNeg(0,settings.sp_structs[0].coolOn,settings.sp_structs[0].coolOff);
            if(val == OFF && (ds[0].pvErr <= settings.sp_structs[0].alarm)) 
                    val = RelayNeg(1,settings.sp_structs[1].coolOn,settings.sp_structs[1].coolOff); // если холодно то не открываем заслонку.
            if(val == ON){EXTRA1 = PCF_ON; pvFlap = 100;} else if(val == OFF){EXTRA1 = PCF_OFF; pvFlap = settings.sp_structs[0].state;}
            // MYDEBUG_PRINT("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ:"); MYDEBUG_PRINTLN(EXTRA1 ? "OFF" : "ON");
          }
        }
      //------------------------- ПОЛОЖЕНИЕ ЗАСЛОНКИ ---------------------------
        // setflap();                            // задание положения заслонки 
        // MYDEBUG_PRINT("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ:"); MYDEBUG_PRINTLN(EXTRA1);

      //------------------ ПОВОРОТ ЛОТКОВ асимметричный режим ----------------------------
      if(settings.sp_structs[1].timer && TURN){// только при sp[1].timer>0 -> асимметричный режим
        TURNSECOND = ON;
        if(--pvTimer==0){
          MYDEBUG_PRINTLN("асимметричный режим: TURN = OFF;");
          pvTimer = settings.sp_structs[0].timer; 
          TURN = PCF_OFF; TURNSECOND = OFF;
          writePCF8574(portOut.value);
        }
      } else {
        TURNSECOND = OFF;
      }
      //--------------------------------- АВАРИЯ ---------------------------------
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
        writePCF8574(portOut.value);
      }

      /* pctHeater = constrain(heaterValue, 0, 255);
      pctHeater = map(pctHeater,0,255,0,100);
      pctHimidifier = constrain(humidiValue, 0, 255);
      pctHimidifier = map(pctHimidifier,0,255,0,100);
      DEBUG_SPRINTF(displStr,"T0=%5.1f; T1=%5.1f; OUT=0x%02x; PW=%u; HU=%u; Tim=%u;  ERR=0x%02x; time: %u;",
        (float)ds[0].pvT/10,(float)ds[1].pvT/10,portOut.value,pctHeater,pctHimidifier,pvTimer,errorsFlag.value,halfSecond);
      MYDEBUG_PRINTLN(displStr);
      // printBinary(portOut.value);
      MYDEBUG_PRINTLN("==============================================================================");
      MYDEBUG_PRINTLN();
       */
      OutStatusLed();  // для HTML страницы
    }//============================== КОНЕЦ СЕКУНДЫ =================================
    // DateTime now = rtc.now();
    #ifdef LED_DISPLAY
      ledSet();     // светодиоды панели
    #endif
    writePCF8574(portOut.value);
  }//============================== КОНЕЦ ПОЛ-СЕКУНДЫ =================================
    
  if (now - lastSendTime > interval) {
    if(earlyMode != mode){
      // Serial.printf("mode:%d; seconds:%d; All time:%ld; \n", mode, seconds, allTime);
      earlyMode = mode;
    }
    lastSendTime = now;
    // Serial.print("Free heap size: ");
    // Serial.println(system_get_free_heap_size());

    if(seconds==0 && mode == READDEFAULT) {mode = READEEPROM; interval = INTERVAL_1000;}
    else if(tableData[0][0]==0 && settings.sp_structs[1].state) {mode = READPROG; interval = INTERVAL_1000; quarter = GET_PROG1;}
    seconds += interval/1000;
    allTime += interval/1000;
    tmrTelegramOff -= interval/1000;  // if you use HTML telegram does not work (5 min.)
    // int16_t lostHeapSize = ESP.getFreeHeap()-begHeapSize;
    // if(lostHeapSize != previousHeapSize){
    //   previousHeapSize = lostHeapSize;
    //   Serial.printf("Lost heap size: %d bytes\n", lostHeapSize);
    // }
    /* switch (mode){
      case READEEPROM: getData(GET_EEPROM); break;
      case READPROG:   getData(quarter); break;
      case SAVEEEPROM: if(++tmrResetMode > 60) tmrResetMode = 0; mode = READDEFAULT; interval = INTERVAL_4000; break;
      case SAVEPROG:   if(++tmrResetMode > 60) tmrResetMode = 0; mode = READDEFAULT; interval = INTERVAL_4000; break;
      default: getData(GET_VALUES); break;
    } */
  }
}
//----------------------------------- loop() ----------------------------------------------------------------------

#ifdef LED_DISPLAY
void ledSet(void){
    byte led = 0;
    if(!(portOut.value&2)) led |= 1;
    if(!(portOut.value&4)) led |= 2;
    if(!TURN) led |= 4;
    if(!EXTRA1) led |= 8;
    if(!EXTRA2) led |= 0x10;
    if(!EXTRA3) led |= 0x20;
    for (uint8_t i = 0; i < 6; i++){
        module.setLED(led&1, i);
        led >>= 1;
    }
}
#endif

// Функция для записи байта на PCF8574
byte writePCF8574(byte data) {
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(data);
  byte error = Wire.endTransmission();
  if (error == 0) {
    //MYDEBUG_PRINT("Data written: 0b");
    //printBinary(data);
    //MYDEBUG_PRINTLN();
  } else {
    MYDEBUG_PRINT("\nError writing to PCF8574. Error code: ");
    MYDEBUG_PRINTLN(error);
  }
  return error;
}

// Функция для чтения байта с PCF8574
byte readPCF8574() {
  Wire.requestFrom(PCF8574_ADDRESS, 1); // Запросить 1 байт данных
  if (Wire.available()) {
    return Wire.read();
  } else {
    MYDEBUG_PRINTLN("Error reading from PCF8574: No data available.");
    return 0xFF; // Возвращаем 0xFF в случае ошибки (можно выбрать другое значение)
  }
}

// Вспомогательная функция для печати байта в двоичном формате
/* void printBinary(byte inByte) {
  for (int b = 7; b >= 0; b--) {
    switch (b) {
    case 7: MYDEBUG_PRINT("N7="); break;
    case 6: MYDEBUG_PRINT("N6="); break;
    case 5: MYDEBUG_PRINT("E3="); break;
    case 4: MYDEBUG_PRINT("E2="); break;
    case 3: MYDEBUG_PRINT("E1="); break;
    case 2: MYDEBUG_PRINT("TU="); break;
    case 1: MYDEBUG_PRINT("HU="); break;
    case 0: MYDEBUG_PRINT("HE="); break;
    }
    if(b>1) MYDEBUG_PRINT(bitRead(inByte, b)? "OF" : "ON");
    else MYDEBUG_PRINT(bitRead(inByte, b)? "ON" : "OF");
    MYDEBUG_PRINTLN();
  }
} */


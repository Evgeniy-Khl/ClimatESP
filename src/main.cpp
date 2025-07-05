
#include "main.h"
#include "my_settings.h"
char displStr[100];


PIDController pid[2];
SoftwarePWMBit heaterPwm(&portOut.value, 0); 
SoftwarePWMBit humidiPwm(&portOut.value, 1);

RTC_DS3231 rtc;                     // Создаем объект RTC для DS3231

OneWire oneWire(ONE_WIRE_BUS_PIN);  // Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
DallasTemperature sensors(&oneWire);// Передаем ссылку на объект oneWire в конструктор DallasTemperature
#ifdef LED_DISPLAY
  TM1638 module(13, 14, 12);    // Создаем объект module для TM1638
#else

#endif
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);       // Инициализация последовательного порта для отладки
  #endif
  //---------------------------- инициализация Конфигурации -----------------------------------
  #ifdef LED_DISPLAY
    pinMode(BEEP_PIN, OUTPUT);    // Настраиваем пин бипера как выход
    initLedConfig();
  #else

  #endif
  pvTimer = settings.sp_structs[0].timer;                  // инициализация времени выключенного состояния таймера
  pvAeration = settings.sp_structs[0].aeration;            // инициализация ПАУЗы ПРОВЕТРИВАНИЯ (минут)
  portOut.value = 0;
  heaterPwm.write(heaterValue);
  humidiPwm.write(humidiValue);
}

void loop() {
  //--------------------------- УПРАВЛЕНИЕ СИМИСТОРОМ ---------------------------------
  bool hasChanged = false;
  long now = millis();
  hasChanged |= heaterPwm.update();
  // hasChanged |= humidiPwm.update();
  if(hasChanged){
    // writePCF8574(portOut.value);
    // sprintf(displStr,"*** NOW = %lu; VAL = %u; **",now,portOut.value);
    // DEBUG_PRINTLN(displStr);
  }
  //============================= ПРОВЕРКА по таймеру =================================
  
  
  #ifdef LED_DISPLAY
    if(now - counter10 > 10){
      counter10 = now;
      if(beepOn) beepOn--; else digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    }
    if(now - counterWait > waitCheckKeyPad){
      counterWait = now;
      byte keys = module.getButtons();
      
      if(lastKey == keys && keys > 0){
        checkkey(keys);
        if(numSetup == 0) ledDispl(displNum);
        else display_setup();
        module.setDisplay(data, 8);
      } 
      else if(keys == 0) waitCheckKeyPad = WAITCHECKKEYPAD ;
      else lastKey = keys;

      sprintf(displStr,"--- NOW = %lu; KEY = %u; wChKeyPad = %u; SetN = %u; BUFF=%u ---",now,keys,waitCheckKeyPad,numSetup,editBuff);
      DEBUG_PRINTLN(displStr);
      // light the first 4 red LEDs and the last 4 green LEDs as the buttons are pressed
      // module.setLEDs(((keys & 0xFF) << 8) | (keys & 0xFF));
    }
  #endif
  //============================= НОВАЯ СЕКУНДА =================================

  if(now - counter1s > 500){
    counter1s = now; 
    // errorsFlag.value = 0;
    if(++halfSecond > 119) halfSecond = 0; 
    if(resetDispl) --resetDispl;
    else if(numSetup) saveset();  // сохраняем установки
    else displNum = 0;            // возврат к главному дисплею
    if(halfSecond & 2){
      //------------------------ ЗНАЧЕНИЯ ТЕМПЕРАТУРЫ --------------------------
      #ifndef DEBUG  
        temperature_check();
      
        if (HIH5030){
          uint16_t adc=1024;
          pvVadcRH = adc;//lowPassF2(adc);           // относительная влажность в Vadc ??????????????????????????????????????????
          if (pvVadcRH>80) pvRH = valDcToRH(pvVadcRH); // относительная влажность в %
          else pvRH = 1990;
        } else {
          uint8_t valTable = tableRH(ds[0].pvT, ds[1].pvT);               // если отсутствует HIH4000 то ...
          if(valTable>100) pvRH = 999; else pvRH = valTable;
        }
      #else
        //-----температура воздуха------
        heaterValue = UpdatePID(0);            // ПИД нагреватель
        humidiValue = UpdatePID(1);            // ПИД увлажнитель
        dpv0 = heaterValue/100;
        ds[0].pvT += dpv0;
        dpv1 = pid[1].pPart/255 + pid[1].iPart/100;
        ds[1].pvT += dpv1;
        //------
        DEBUG_PRINTLN();
        sprintf(displStr,"=== Sek = %u; ResD = %u; DspN = %u; SetN = %u ===",halfSecond/2,resetDispl,displNum,numSetup);
        DEBUG_PRINTLN(displStr);
        sprintf(displStr,"Пропорц.0= %g  Ітеграл.0= %g", pid[0].Kp,pid[0].Ki);
        DEBUG_PRINTLN(displStr);
        // sprintf(displStr,"Пропорц.1= %g  Ітеграл.1= %g", pid[1].Kp,pid[1].Ki);
        // DEBUG_PRINTLN(displStr);
        sprintf(displStr,"error0 = %i", ds[0].pvErr);
        DEBUG_PRINTLN(displStr);
        sprintf(displStr,"pP0 = %g; iP0 = %g; out = %g;",pid[0].pPart,pid[0].iPart,pid[0].pPart+pid[0].iPart);
        DEBUG_PRINTLN(displStr);
        sprintf(displStr,"dpv0 = %g;",dpv0);
        DEBUG_PRINTLN(displStr);
        sprintf(displStr,"heaterValue = %u; humidiValue = %u",heaterValue,humidiValue);
        DEBUG_PRINTLN(displStr);
        sprintf(displStr,"T0 = %5.1f; T1 = %5.1f",(float)ds[0].pvT/10,(float)ds[1].pvT/10);
        DEBUG_PRINTLN(displStr);
        Serial.flush();
        //------
      #endif
        if(!COOLING){  //-------------- нормальная работа -------------------------
          DEBUG_PRINTLN("ПИД нагреватель и реле.");
          switch (settings.sp_structs[0].mode) {
              uint8_t val;
              case 0:
                heaterValue = UpdatePID(0);            // ПИД нагреватель
                humidiValue = UpdatePID(1);            // ПИД увлажнитель
                break;
              case 1:
                val = RelayPos(0,2);
                switch (val){
                    case ON: heaterValue = TRIACON; break;
                    case OFF: heaterValue = OFF;    break;
                }
                humidiValue = UpdatePID(1);            // ПИД увлажнитель
                break;
              case 2:
                heaterValue = UpdatePID(0);            // ПИД нагреватель
                val = RelayPos(1,3);
                switch (val){
                    case ON: humidiValue = TRIACON; break;
                    case OFF: humidiValue = OFF;    break;
                }
                break;
              case 3:
                val = RelayPos(0,2);
                switch (val){
                    case ON: heaterValue = TRIACON; break;
                    case OFF: heaterValue = OFF;    break;
                }
                val = RelayPos(1,3);
                switch (val){
                    case ON: humidiValue = TRIACON; break;
                    case OFF: humidiValue = OFF;    break;
                }
                break;
              case 4:
                heaterValue = UpdatePID(0);           // ПИД нагреватель
                OutPulse();                           // импульсное управление увлажнителем
                break;
          }
        } else {heaterValue = 0; displPower = 0;}      //-- идет ОХЛАЖДЕНИЕ!--
        if(settings.sp_structs[1].mode == 1 && !REACHED0) humidiValue=OFF; // задержка регулирования по 2 каналу до прогрева инкубатора
        heaterPwm.write(heaterValue);
        humidiPwm.write(humidiValue);

        if(!COOLING){  //-------------- нормальная работа -------------------------
          DEBUG_PRINTLN("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ.");
          //------ КАНАЛ ВСПОМОГАТЕЛЬНОГО НАГРЕВАТЕЛЯ -------------------------------------------------
          if(ERROR1 == 0){
            if (ds[0].pvErr >= settings.sp_structs[0].auxiliary) EXTRA2 = ON;       // включить вспомогательны нагреватель
            else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) EXTRA2 = OFF; // отключить вспомогательны нагреватель
          } else EXTRA2 = OFF;                                                      // отключить вспомогательны нагреватель
          //------ ОХЛАЖДЕНИЕ  ОСУШЕНИЕ ---------------------------------------------------------------
          uint8_t val = RelayNeg(0,settings.sp_structs[0].coolOn,settings.sp_structs[0].coolOff);
          if(val == OFF && (ds[0].pvErr <= settings.sp_structs[0].alarm)) 
                  val = RelayNeg(1,settings.sp_structs[1].coolOn,settings.sp_structs[1].coolOff); // если холодно то не открываем заслонку.
          if(val == ON){EXTRA1 = ON; pvFlap = 100;} else if(val == OFF){EXTRA1 = OFF; pvFlap = settings.sp_structs[0].state;}
          //------ АВАРИЙНОЕ ВЫКЛЮЧЕНИЕ ---------------------------------------------------------------
          if(settings.sp_structs[0].extendMode&1){    // [0]-0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
            uint8_t val = RelayNeg(0,settings.sp_structs[0].alarm,settings.sp_structs[0].spT); // канал 5 АВАРИЙНОЕ ВЫКЛЮЧЕНИЕ.
            if(val == ON) EXTRA3 = ON;                // включить канал 5
            else if(val == OFF) EXTRA3 = OFF;         // отключить канал 5
          }
        //------- ПРОВЕТРИВАНИЕ ----------------------------------------------------------------------    
          if(AERATION){     // Идет ПРОВЕТРИВАНИЕ !
            EXTRA1 = ON; pvFlap = 100; beepOn = 10;
            if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; AERATION =0;}
          }
          // if(setup==0) alarm();
        }
        // setflap();                            // задание положения заслонки 
        // if((setup+setprgday)==0) display(displmode);// вывод на дисплей

        //-------------------------
    }
    // DateTime now = rtc.now();
    #ifdef LED_DISPLAY
      if(numSetup == 0) ledDispl(displNum);
      else display_setup();
      module.setDisplay(data, 8);

    #endif
    //-----------------------------------------------------------------------------

    // -- Пример 1: Управление выходами PCF8574 (как светодиодами) ---
    // writePCF8574(now.second()%10);
    /* -- Пример 2: Чтение входов PCF8574 ---
          Чтобы читать пины как входы, сначала запишите в них 0xFF (все единицы),
          чтобы перевести их в режим "квази-входа" с высоким импедансом.
          Если к пину ничего не подключено или подключено к VCC, вы прочитаете '1'.
          Если пин замкнут на GND, вы прочитаете '0'. 
    writePCF8574(0x80); // Устанавливаем  пин в режим "квази-входа"
    delay(100); // Небольшая задержка для стабилизации
    byte inputData = readPCF8574();
    // Пример проверки состояния конкретного пина (например, P8)
    if (!(inputData & 0x80)) { // Если P8 равен 0
      DEBUG_PRINTLN("Pin P8 is LOW");
    } else {
      DEBUG_PRINTLN("Pin P8 is HIGH");
    }
    */
    //==================== НОВАЯ МИНУТА =======================================
    if(halfSecond == 0){
      //---------------------------- ПОВОРОТ ЛОТКОВ ----------------------------
        if(settings.sp_structs[0].timer) rotate_trays();
      //---------------------------- ПРОВЕТРИВАНИЕ !! --------------------------
        if(!AERATION && !COOLING && settings.sp_structs[1].aeration){
          if(--pvAeration == 0){
            pvVenting = settings.sp_structs[1].aeration; AERATION = 1; EXTRA1 = ON;
          //  if((relayMode & 4) && checkDry==0) {pwTriac1=maxRun; CN2 = CN2ON;}// принудительный впрыск воды!!!
          }
        } else if(COOLING){
          EXTRA1 = ON; pvFlap = 100; beepOn = 50;
          if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; COOLING = 0;}
          // if(extendMode&1) BREAK=ON; 
        }
    }//==================== КОНЕЦ МИНУТЫ  ===================================
  }//====================== КОНЕЦ СЕКУНДЫ ===================================
}//-------------------------- loop() ---------------------------------------

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



#include "main.h"
#include "my_settings.h"
char displStr[200];


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
  hasChanged |= humidiPwm.update();
  if(hasChanged){
    // writePCF8574(portOut.value);
    // sprintf(displStr,"*** NOW = %lu; VAL = %u; **",now,portOut.value);
    // DEBUG_PRINTLN(displStr);
  }
  //============================= ПРОВЕРКА по таймеру =================================
  
  
  #ifdef LED_DISPLAY
  //-------------------------------------------- 10 mSec. --------------------------------------
    if(now - counter10 > 10){
      counter10 = now;
      if(beepOn) beepOn--; else digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
      if(settings.sp_structs[0].mode == 4 && --pvPulse == 0){ // импульсный режим увлажнения
        HUMIDI = OFF;
        writePCF8574(portOut.value);
      }
      keys = module.getButtons();
      if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}
    }

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
      if(numSetup == 0) ledDispl(displNum);
      else display_setup();
      module.setDisplay(data, 8);
      
    #endif
    if(halfSecond & 2){
      errorsFlag.value = 0;
  //================================ НОВАЯ СЕКУНДА =================================
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
          // DEBUG_PRINTLN("ПИД нагреватель и реле.");
          //--- режим реле = 0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]; 4-импульс ---
          switch (settings.sp_structs[1].mode) {
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
                if (pvPeriod) --pvPeriod;
                else {
                  pvPeriod = settings.sp_structs[1].pulse;  // начало нового периода
                  if(pvPulse) HUMIDI = ON;                  // включить канал 2 (импульсный режим)
                };
                break;
          }
        } else {heaterValue = 0; displPower = 0;}      //-- идет ОХЛАЖДЕНИЕ!--
        if(settings.sp_structs[0].mode == 1 && !REACHED0) humidiValue=OFF; // задержка регулирования по 2 каналу до прогрева инкубатора
        // heaterPwm.write(heaterValue);
        // humidiPwm.write(humidiValue);

        if(!COOLING){  //-------------- нормальная работа -------------------------
          // DEBUG_PRINTLN("ОХЛАЖДЕНИЕ  ОСУШЕНИЕ.");
          //------ КАНАЛ ВСПОМОГАТЕЛЬНОГО НАГРЕВАТЕЛЯ -------------------------------------------------
          if(ERROR1 == 0){
            if(ds[0].pvErr >= settings.sp_structs[0].auxiliary) EXTRA2 = ON;       // включить вспомогательны нагреватель
            else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) EXTRA2 = OFF; // отключить вспомогательны нагреватель
          } else EXTRA2 = OFF;                                                      // отключить вспомогательны нагреватель
          //------ ОХЛАЖДЕНИЕ  ОСУШЕНИЕ ---------------------------------------------------------------
          uint8_t val = RelayNeg(0,settings.sp_structs[0].coolOn,settings.sp_structs[0].coolOff);
          if(val == OFF && (ds[0].pvErr <= settings.sp_structs[0].alarm)) 
                  val = RelayNeg(1,settings.sp_structs[1].coolOn,settings.sp_structs[1].coolOff); // если холодно то не открываем заслонку.
          if(val == ON){EXTRA1 = ON; pvFlap = 100;} else if(val == OFF){EXTRA1 = OFF; pvFlap = settings.sp_structs[0].state;}
          
        //------------------------- ПРОВЕТРИВАНИЕ -----------------------------
          if(AERATION){     // Идет ПРОВЕТРИВАНИЕ !
            EXTRA1 = ON; pvFlap = 100; beeperOn(10);
            if(--pvVenting == 0){pvAeration = settings.sp_structs[0].aeration; AERATION =0;}
          }
        }
      
      //------------------------- ПОЛОЖЕНИЕ ЗАСЛОНКИ ---------------------------
        // setflap();                            // задание положения заслонки 

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
      byte led = portOut.value;
      for (uint8_t i = 0; i < 6; i++){
        module.setLED(led&1, i);
        led >>= 1;
      }
  }//=================== КОНЕЦ ПОЛ СЕКУНДЫ ===================================
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


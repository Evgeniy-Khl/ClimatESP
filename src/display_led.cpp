#include "main.h"
#include "display_led.h"

uint8_t data[] = {
  0b00111111, // 0
  0b01011110, // d
  0b00000110, // 1
  0b11101101, // 5.
  0b01011011, // 2
  0b01101101, // 5
  0,          // blank
  0           // blank
};


//--------- ОСНОВНОЙ ЭКРАН ----------------------
// void ledDispl(void){
//   data[0] = NUMBER_FONT[ds[0].pvT/100];
//   data[1] = NUMBER_FONT[(ds[0].pvT%100)/10] | 0b10000000;
//   data[2] = NUMBER_FONT[ds[0].pvT%10];
//   data[3] = NUMBER_FONT[ds[1].pvT/100];
//   data[4] = NUMBER_FONT[(ds[1].pvT%100)/10] | 0b10000000;
//   data[5] = NUMBER_FONT[ds[1].pvT%10];

//   data[6] = NUMBER_FONT[seconds/10];
//   data[7] = NUMBER_FONT[seconds%10];

//   module.setDisplay(data, 8);
// }

void displ_top(signed int val, unsigned char comma){
 unsigned char i, neg=0;
 if((ERROR1 || ERROR4) && (seconds & 1)){for (i=0; i<3; i++) data[i]=BL;} // мигают цифры
 else
  {
   if (comma) comma=0x80;
   if (val<0) {neg=1; val=-val;}
   if (val<1000)
    {
     if (neg)
      {
       if (val<100)
        {
          data[0] = DEF;
          data[1] = NUMBER_FONT[((val/10)%10)&0x0F]|comma; // запятая
          data[2] = NUMBER_FONT[(val%10)&0x0F];
        }
       else
        {
          data[0] = DEF;
          data[1] = NUMBER_FONT[(val/100)&0x0F];
          data[2] = NUMBER_FONT[((val/10)%10)&0x0F];
        };
      }
     else
      {
       data[0] = NUMBER_FONT[(val/100)&0x0F];
       data[1] = NUMBER_FONT[((val/10)%10)&0x0F]|comma; // запятая
       data[2] = NUMBER_FONT[(val%10)&0x0F];
      };
    }
   else
    {
     data[0] = 0x06;// -> 1
     data[1] = 0x6f;// -> 9
     data[2] = 0x6f;// -> 9
    };
  }
}

void displ_bot(signed int val, unsigned char comma){
 unsigned char i, neg=0;
 if((ERROR10)&&(seconds & 1)){for (i=3; i<6; i++) data[i]=BL;} // мигают цифры
 else
  {
   if (comma) comma=0x80;
   if (val<0) {neg=1; val=-val;}
   if (val<1000)
    {
     if (neg)
      {
       if (val<100)
        {
          data[3] = DEF;
          data[4] = NUMBER_FONT[((val/10)%10)&0x0F]|comma; // запятая
          data[5] = NUMBER_FONT[(val%10)&0x0F];
        }
       else
        {
          data[3] = DEF;
          data[4] = NUMBER_FONT[(val/100)&0x0F];
          data[5] = NUMBER_FONT[((val/10)%10)&0x0F];
        };
      }
     else
      {
       data[3] = NUMBER_FONT[(val/100)&0x0F];
       data[4] = NUMBER_FONT[((val/10)%10)&0x0F]|comma; // запятая
       data[5] = NUMBER_FONT[(val%10)&0x0F];
      };
    }
   else
    {
     data[3] = 0x06;// -> 1
     data[4] = 0x6f;// -> 9
     data[5] = 0x6f;// -> 9
    };
  }
}

void clr_top(void){
  unsigned char byte;
  for (byte=0; byte<3; byte++) data[byte]=BL;
}

void clr_bot(void){
  unsigned char byte;
  for (byte=3; byte<6; byte++) data[byte]=BL;
}

void displ_67(signed int val, unsigned char mode)
{
  unsigned char neg=0, comma=0;
  if(mode==1) comma=0x80;
  if(val<0) {neg=1; val=-val;}
  if(val<1000)
   {
    if(neg)
     {
       if(val<100)
        {
          data[6] = DEF;
          data[7] = NUMBER_FONT[((val/10)%10)&0x0F]|comma; // запятая
          data[8] = NUMBER_FONT[(val%10)&0x0F];
        }
       else
        {
          data[6] = DEF;
          data[7] = NUMBER_FONT[(val/100)&0x0F];
          data[8] = NUMBER_FONT[((val/10)%10)&0x0F];
        };
     }
    else if(val<999)
     {
        
       data[6] = NUMBER_FONT[(val/100)&0x0F];
       data[7] = NUMBER_FONT[((val/10)%10)&0x0F]|comma; // запятая
       data[8] = NUMBER_FONT[(val%10)&0x0F];
       if(val<100)
        {
         switch (mode)
           {
             case ERRORS:  data[6] = EE; break;
             case DAY:     data[6] = DD; break;
             case COOL:    data[6] = GR; break;
             default:      data[6] = BL;
           }
        }
     }
    else {data[6] = EE; data[7] = RR; data[8] = RR;}; // Err
   }
  else {data[6] = 0x06; data[7] = 0x6f; data[8] = 0x6f;};// -> 199
}

//===================== ledDisplay ========================
void ledDispl(unsigned char mode){
  switch (mode){
        //-------------t0;----------------------------RH;            /              t1; --------------------Timer;-------------------
    case 0:
          displ_top(ds[0].pvT,COMMA); 
          if(HIH5030) displ_bot(pvRH,COMMA); else displ_bot(ds[1].pvT,COMMA);
          if(errorsFlag.value) displ_67(errorsFlag.value, ERRORS); 
          else if(COOLING || AERATION) displ_67(pvVenting, COOL);
          // else if(programm) displ_678(date,DAY); 
          else if(HIH5030) displ_67(pvTimer, NOCOMMA); 
          else displ_67(pvRH, NOCOMMA);
      break;
       //-------------------t1;----------------------tNTC;--------------------"F2"---------
    case 1: if(HIH5030) displ_top(ds[1].pvT, COMMA); 
            else displ_top(pvTimer, NOCOMMA); 
            // displ_bot(pvTTriac, NOCOMMA); 
            data[6]=FF; data[7]=0x5b; 
      break;
       //-------------------Flap;--------------------date;--------------------"F3"---------
    case 2: displ_top(settings.sp_structs[0].state, NOCOMMA); 
            displ_bot(0,NOCOMMA); data[6]=FF; data[7]=0x4f; 
      break;
       //---------------уставка t0;-------------------------уставка RH;-----------------------уставка t1-----------------"F4"---------
    case 3: displ_top(settings.sp_structs[0].spT, COMMA); 
            if(HIH5030) displ_bot(settings.sp_structs[1].spRH, COMMA); 
            else displ_bot(settings.sp_structs[1].spT, COMMA);
            data[6]=FF; data[7]=0x66;
      break;
 }
 if (OVERHEAT)  {
    data[6] = PE; data[7] = GE;
    if(seconds & 1) {for (uint8_t i=0; i<8; i++) data[i] = BL;}; // мигание дисплея при перегреве симистора
  };
}

//==================== Setup ========================
void display_setup(uint8_t mode){
  // errorsFlag = 0;
  if(editBuff>999) editBuff=999; else if(editBuff<-99) editBuff=-99;
  if(mode==1||mode==7||mode==9||mode==14||mode==23){
    displ_top(editBuff,COMMA); clr_bot();
  }                //Верхний дисплей + Запятая
  else if(mode==2||mode==8||mode==10||mode==13||mode==15||mode==20||mode==21){
    clr_top(); displ_bot(editBuff,COMMA);
  }//Нижний дисплей + Запятая
  else if(mode==22){
    clr_top(); displ_bot(editBuff,NOCOMMA);
  }                                              //Нижний дисплей
  else {
    if(editBuff<0) editBuff=0; displ_top(editBuff,NOCOMMA); clr_bot();
  }                                         //Верхний дисплей 
}

//============================== Config ========================================
void initLedConfig(void){
  char displStr[65];
//--------- инициализация FS -----------------------------------------
  if (!LittleFS.begin()) {
    DEBUG_PRINTLN("Flash FS initialisation failed!");
    data[6] = NUMBER_FONT[14];  // "E"
    saveConfig();  // значения по умолчанию
    delay(3000);
  }
//--------- Загрузка конфигурации --------------------------------------------
  if(LittleFS.exists("/setpoint.json")){
    if(!loadConfig()){
      DEBUG_PRINTLN("Конфігурація не завантажена!");
      data[6] = NUMBER_FONT[12];  // "C"
      saveConfig();  // значения по умолчанию
      delay(3000);
    }
  }
  else {
    saveConfig();  // значения по умолчанию
    DEBUG_PRINTLN("Конфігурація за замовчуванням!");
    data[6] = NUMBER_FONT[10];  // "A"
    delay(3000);
  }
  DEBUG_PRINTLN("\n>> Итоговые значения после загрузки из FS:");
  #ifdef DEBUG
    printConfig();
  #endif
  //--------- инициализация PID --------------------------------------------
  PID_Init(&pid[0], settings.sp_structs[0].Kp, settings.sp_structs[0].Ki);
  PID_Init(&pid[1], settings.sp_structs[1].Kp, settings.sp_structs[1].Ki);

  sprintf(displStr,"Пропорц.0= %g  Ітеграл.0= %g", pid[0].Kp,pid[0].Ki);
  DEBUG_PRINTLN(displStr);
  sprintf(displStr,"Пропорц.1= %g  Ітеграл.1= %g", pid[1].Kp,pid[1].Ki);
  DEBUG_PRINTLN(displStr);
  
  //------------------------------------------------------------------------
  /* DEBUG_PRINTLN("\n");
  uint32_t realSize = ESP.getFlashChipRealSize(); // Получаем реальный размер flash
  uint32_t ideSize = ESP.getFlashChipSize();    // Получаем размер, установленный в IDE
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u bytes\n\n", realSize);

  Serial.printf("Flash ide  size: %u bytes\n", ideSize);
  Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

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

  Wire.begin();               // Инициализация I2C (SDA, SCL по умолчанию для ESP8266 - GPIO4, GPIO5)
  // Wire.begin(D2, D1);      // Если вы хотите использовать другие пины для I2C (например, D2 для SDA, D1 для SCL)
  //--------------------- Инициализация PCF8574 ----------------------------------
  /* Пример: Установить все пины PCF8574 как выходы и выключить их (записать 0)
            Для PCF8574, чтобы использовать пин как "выход", мы просто записываем в него значение.
            Чтобы использовать пин как "вход", мы записываем в него '1' (высокий уровень),
            а затем читаем состояние. Внутренние подтягивающие резисторы слабые. 
  */
  writePCF8574(0x00);         // Установить все пины в LOW (если они используются как выходы)

  //---------- Инициализация DS3231 ----------------------------------------
  if(!rtc.begin()) {
    DEBUG_PRINTLN("RTC NOT found!");
    data[7] = NUMBER_FONT[9];   // "9"
  }
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
  data[0] = NUMBER_FONT[numberOfDevices]; // отображение числа датчиков на дисплее
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
  module.setDisplay(data, 8); // Вывод на дисплей "2d1 | 5.12"
  delay(3000);
}
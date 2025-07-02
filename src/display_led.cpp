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
void ledDispl(void){
  data[0] = NUMBER_FONT[ds[0].pvT/100];
  data[1] = NUMBER_FONT[(ds[0].pvT%100)/10] | 0b10000000;
  data[2] = NUMBER_FONT[ds[0].pvT%10];
  data[3] = NUMBER_FONT[ds[1].pvT/100];
  data[4] = NUMBER_FONT[(ds[1].pvT%100)/10] | 0b10000000;
  data[5] = NUMBER_FONT[ds[1].pvT%10];

  data[6] = NUMBER_FONT[seconds/10];
  data[7] = NUMBER_FONT[seconds%10];

  module.setDisplay(data, 8);
}

void menu_1(){
  
}

void menu_2(){
  
}

void menu_3(){
  
}

void menu_4(){
  
}

void calcDisplay(const char* txt){
  
}

void drawKeypad_longName_7(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  
}

void drawKeypad_longName_12(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  
}

void drawKeypad(const char* keyLabel[], uint16_t keyColor[]){
  
}

uint16_t lampUpdate(uint16_t xpos, uint16_t ypos){
    return false;
}

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
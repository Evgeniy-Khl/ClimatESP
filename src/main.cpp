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
DateTime now;

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
  module.setDisplay(data, 8);                       //"ooo ooo oo"
  if(temp){
     dataLed[5] = 1;                                // ошибка writePCF8574
  }
  //----------------------------------- MOUNTING FS ----------------------------------------
  MYDEBUG_PRINTLN("mounting FS...");
  bool lFS = LittleFS.begin();
  if(lFS) {
    MYDEBUG_PRINTLN("mounted file system");
    listFilesAndSizes();
    //--------------------------------- clean LittleFS, for testing -----------------------
    // **Здесь вы можете разместить LittleFS.format();  но ОЧЕНЬ ВАЖНО ПОНИМАТЬ КОГДА ЭТО ДЕЛАТЬ!**
    // Например, вы можете отформатировать файловую систему только при первом запуске или при определенном условии.
    // **ВНИМАНИЕ: Раскомментирование следующей строки приведет к форматированию LittleFS при каждом запуске!**
    // Проверка и форматирование, если необходимо
    // if (LittleFS.format()) {
    //   MYDEBUG_PRINTLN("LittleFS formatted successfully");
    // } else {
    //   MYDEBUG_PRINTLN("Failed to format LittleFS");
    // }
    //--------------------------------------------------------------------------------------
    //--------------------- checkSetpoint ----------------------------------
    dataLed[2] = checkSetpoint();
    dataLed[3] = checkConfig();
  } else {
    MYDEBUG_PRINTLN("failed to mount FS");
    dataLed[4] = 1;
  }
  //---------------------------- инициализация WiFiManager -----------------------------------
  if(settings.sp_structs[0].special & 0x03) initWiFiManag();
  else MYDEBUG_PRINTLN("Запрет на подключение к WiFi! Продолжаем работу в оффлайн-режиме.");
  //------------------------------------------------------------------------------
  PID_Init(&pid[0], settings.sp_structs[0].Kp, settings.sp_structs[0].Ki);
  PID_Init(&pid[1], settings.sp_structs[1].Kp, settings.sp_structs[1].Ki);
  //---------- Инициализация DS3231 ----------------------------------------
  if(rtc.begin()){
    RTCENABLE = 1;
    if(rtc.lostPower()) {                     // у RTC села батарейка
        MYDEBUG_PRINTLN("RTC lost power! Текущая программа обнулена!");
        dataLed[1] = 1;                       // RTC lost power
        // Установка времени: 1 год, 1 месяц, 1 день, 00:00:00
        rtc.adjust(DateTime(2025, 1, 1, 0, 0, 0));
        eepromWriteByte(STARTINCUBADRES, 0);  // если и был старт инкубации то теперь сброшен
        settings.sp_structs[1].state = 0;     // [1]-программа текущая обнулена
        // saveSetpoint();  //????????????????????
    } else {
      if(eepromReadByte(STARTINCUBADRES)) INCUBATION = 1; // установим флаг
    }
  } else dataLed[0] = 1;      // DS3231 не инициализирован
  //------------------------------------------------------------------------------
  // testProgs();              // тест
  //----------------------- определяем какой датчик подключен --------------------------------
  sensorType();
  //------------------------------------------------------------------------------------------
  digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
  pinMode(BEEP_PIN, OUTPUT);    // Настраиваем пин бипера как выход только для LED

  displ_IP();       // ИНДИКАЦИЯ ОШИБОК и IP адреса 
  clearEEPROM();    // Заполняет нулями 288 записей область памяти в AT24C32, используемую для хранения суточных данных.
  pvTimer = settings.sp_structs[0].timer;                   // инициализация времени выключенного состояния таймера
  pvAeration = settings.sp_structs[0].aeration;             // инициализация ПАУЗы ПРОВЕТРИВАНИЯ (минут)
  heaterPwm.write(heaterValue);
  humidiPwm.write(humidiValue);
  portOut.value = 0xFF;
  delay(3000);
  begHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
  DEBUG_PRINTF("Free heap size: %d\n", begHeapSize);
}

void loop(){
  server.handleClient(); // Обработка входящих запросов
  //--------------------------- УПРАВЛЕНИЕ СИМИСТОРОМ 10 mSec. ---------------------------------
  long nowMillis = millis();
  if(nowMillis - counter10 > 10){
    counter10 = nowMillis;
    hasChanged = false;
    hasChanged |= heaterPwm.update();
    hasChanged |= humidiPwm.update();

    if(hasChanged) {
      writePCF8574(portOut.value);
      ledSet();
    }

    if(beepOn) beepOn--; else digitalWrite(BEEP_PIN, HIGH);   // Выключаем бипер

    if(settings.sp_structs[0].mode == 4 && --pvPulse == 0){   // импульсный режим увлажнения
      humidiValue = TRIACOFF;
      writePCF8574(portOut.value);
      ledSet();
    }

    keys = module.getButtons();
    if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}  // если не удерживается ни одна кнопка то сброс времени ожидания.
  }
  //-------------------------------------------- КЛАВИАТУРА --------------------------------------
    if(nowMillis - counterWait > waitCheckKeyPad){
      counterWait = nowMillis;
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
  //============================= НОВАЯ ПОЛ-СЕКУНДА =================================
  if(nowMillis - counter500 > 500){
    halfSecond++;
    counter500 = nowMillis; 

    if(resetDispl) --resetDispl;
    else if(numSetup) saveset();  // сохраняем установки
    else displNum = 0;            // возврат к главному дисплею

    if(numSetup == 0) ledDispl(displNum); 
    else display_setup();

    module.setDisplay(data, 8);
    ledSet();                     // светодиоды панели
    // writePCF8574(portOut.value);  // ??????????????
  }
  //================================ НОВАЯ СЕКУНДА =================================
  if(nowMillis - counter1000 > 1000){
    counter1000 = nowMillis;
    newSecond();
    heaterPwm.write(heaterValue);
    humidiPwm.write(humidiValue);
    writePCF8574(portOut.value);
    OutStatusLed();               // для HTML страницы
    if(++countSeconds > 0) newMinute();
  }
  //************************************************ TELEGRAM *************************************************/
  if (nowMillis - lastSendTime > interval) {
    if(earlyMode != mode){
      // DEBUG_PRINTF("mode:%d; seconds:%d; All time:%ld; \n", mode, seconds, allTime);
      earlyMode = mode;
    }
    lastSendTime = nowMillis;
    // Serial.print("Free heap size: ");
    // MYDEBUG_PRINTLN(system_get_free_heap_size());

    if(seconds==0 && mode == READDEFAULT) {mode = READEEPROM; interval = INTERVAL_1000;}
    else if(tableData[0][0]==0 && settings.sp_structs[1].state) {mode = READPROG; interval = INTERVAL_1000; quarter = GET_PROG1;}
    seconds += interval/1000;
    allTime += interval/1000;
    tmrTelegramOff -= interval/1000;  // if you use HTML telegram does not work (5 min.)
    // int16_t lostHeapSize = ESP.getFreeHeap()-begHeapSize;
    // if(lostHeapSize != previousHeapSize){
    //   previousHeapSize = lostHeapSize;
    //   DEBUG_PRINTF("Lost heap size: %d bytes\n", lostHeapSize);
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

void ledSet(void){
    byte led = 0;
    if(!TURN) led |= 1;
    if(!(portOut.value&2)) led |= 2;
    if(!(portOut.value&4)) led |= 4;
    if(!EXTRA1) led |= 8;
    if(!EXTRA2) led |= 0x10;
    if(!EXTRA3) led |= 0x20;
    for (uint8_t i = 0; i < 6; i++){
        module.setLED(led&1, i);
        led >>= 1;
    }
}

// Функция для записи байта на PCF8574
byte writePCF8574(byte data) {
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(data);
  byte error = Wire.endTransmission();
  if(error) {
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



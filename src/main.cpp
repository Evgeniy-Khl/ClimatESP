// #include <Arduino.h>
#include <SPI.h>
#include "TFT_eSPI.h"
// #include "Keypad_240x320.h"
// #include "Free_Font_Demo.h"
// #include "tftProcessing.h"
#include "tftArcFill.h"
#include "display.h"
#include "procedure.h"
#include <Wire.h>     // Библиотека для I2C связи
#include <RTClib.h>   // Библиотека для работы с RTC DS3231
// #include <OneWire.h>
// #include <DallasTemperature.h>
#include "AT24C32.h"
// Пин, к которому подключен светодиод (например, D4 на NodeMCU, это GPIO2)
const int ledPin = 2; // GPIO2

// Пин, к которому подключен информационный вывод (DQ) датчика DS18B20
// #define ONE_WIRE_BUS_PIN 0 // используется номер GPIO
// #define MAX_DEVICE 4        // ограничение количества датчиков
// uint8_t numberOfDevices, errDevice[MAX_DEVICE];
// Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
// OneWire oneWire(ONE_WIRE_BUS_PIN);
// Передаем ссылку на объект oneWire в конструктор DallasTemperature
// DallasTemperature sensors(&oneWire);
// Переменная для хранения адреса датчика (если их несколько)
// DeviceAddress sensorAddress;

// Переменные для управления яркостью
int brightness = 0;    // Текущая яркость
int fadeAmount = 5;    // На сколько изменять яркость за один шаг

// Адрес PCF8574. Может быть разным в зависимости от конфигурации A0, A1, A2.
// Стандартные адреса: 0x20-0x27 для PCF8574 и 0x38-0x3F для PCF8574A.
// Уточните адрес вашего модуля. Часто по умолчанию 0x27 или 0x3F.
#define PCF8574_ADDRESS 0x27 // Замените на ваш адрес, если необходимо
long lastMsg = 0, number = 0;
//---------------------------------
char displStr[50];
uint16_t xpos, ypos, txt_height;
uint8_t diplNum=0, seconds=0, pwTriac;
// spT spRH timer alarm coolOn coolOff aeration flapLimit state service pulse mode extendMode Kp Ki Kd
#define FLPCLOSE 0
#define FLPOPEN 255
PIDController pid[2];

Sp sp[2] = {{350,   0, 60, 10, 5, 2, 10, FLPCLOSE, 0, 10, 100, 0,   0, 20, 1, 1}, 
            {300, 650,  0, 15, 5, 2,  0,  FLPOPEN, 0,  5,2000, 0,3000, 20, 1, 1}};
//---------------------------------
Ds ds[2] = {{350,0},{280,0}};
uint16_t set[2] = {385, 305};
int8_t dpv1 = 2;
float flT0=350, dpv0;
//---------------------------------
GrafDispl grafDispl[2] = {
    { 80,80,80, 0, 0},    // Инициализация grafDispl[0]
    {240,80,80, 0, 0},    // Инициализация grafDispl[1]
};
byte writePCF8574(byte data);
byte readPCF8574();
void testAT24C32();
// void printAddress(DeviceAddress deviceAddress);
RTC_DS3231 rtc;               // Создаем объект RTC для DS3231

// Создаем объекты TFT
TFT_eSPI tft = TFT_eSPI(); // Создаем экземпляр библиотеки
// void initTFT(void);
// XPT2046_Touchscreen ts(TOUCH_CS); // Используем только CS
//XPT2046_Touchscreen ts(TOUCH_CS, TIRQ_PIN); // Если используете T_IRQ

void setup() {
  Serial.begin(115200);       // Инициализация последовательного порта для отладки
  //--------- инициализация SPIFFS -----------------------------------------
  if (!SPIFFS.begin()) {
      Serial.println("ERROR file system!");
      tft.setTextColor(TFT_RED, TFT_YELLOW);
      tft.drawString("ERROR file system!", xpos, ypos, 4);
      delay(10000);
      xpos = 0; ypos += 30;
  }
  //--------- инициализация TFT --------------------------------------------
  initMyTFT();
  tft.setTextDatum(TL_DATUM);
  //--------- инициализация Configuration ----------------------------------
  // 1. Показываем начальные значения, заданные в коде
    // Serial.println(">> Начальные значения из кода:");
    // printConfig();
  if(SPIFFS.exists("/setpoint.json")){
      if(loadConfig()){
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Configuration loaded successfully.", xpos, ypos, 2);
      }
      else {
        tft.setTextColor(TFT_YELLOW, TFT_RED);
        tft.drawString("Configuration not loaded!", xpos, ypos, 2);
      }
      xpos = 0; ypos += 20;
  }
  else saveConfig();  // Сохраним эти значения в файл
    /* // 3. Для демонстрации, очищаем структуру в памяти
    Serial.println("\n>> Очищаем структуру в ОЗУ для проверки загрузки...");
    memset(sp, 0, sizeof(sp)); // Заполняем массив нулями
    printConfig(); */
    /* // 4. Загружаем значения из файла обратно в структуру
    Serial.println("\n>> Загружаем данные из файла...");
    loadConfig(); */
    // 5. Показываем результат после загрузки
  Serial.println("\n>> Итоговые значения после загрузки из FS:");
  printConfig();
  //===========================================
  //--------- инициализация PID --------------------------------------------
  PID_Init(&pid[0], sp[0].Kp, sp[0].Ki, sp[0].Kd);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprintf(displStr,"Kp=%g  Ki=%g  Kd=%d", pid[0].Kp,pid[0].Ki,pid[0].Kd);
  tft.drawString(displStr, xpos, ypos, 2);
  xpos = 0; ypos += 20;
  //------------------------------------------------------------------------
  /* Serial.println("\n");
  uint32_t realSize = ESP.getFlashChipRealSize(); // Получаем реальный размер flash
  uint32_t ideSize = ESP.getFlashChipSize();    // Получаем размер, установленный в IDE
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u bytes\n\n", realSize);

  Serial.printf("Flash ide  size: %u bytes\n", ideSize);
  Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

  if (ideSize != realSize) {
    Serial.println("Внимание! Размер Flash, установленный в IDE, не совпадает с реальным!");
  } else {
    Serial.println("Размер Flash в IDE совпадает с реальным.");
  }
  Serial.println(); */
  //------------------------------------------------------------------------------
  pinMode(ledPin, OUTPUT);    // Устанавливаем пин светодиода как выход
  // Можно установить желаемую частоту ШИМ (опционально)
  // analogWriteFreq(1000);   // По умолчанию и так 1000 Гц
  // Можно установить желаемый диапазон (опционально)
  analogWriteRange(255);      // Если хотите диапазон 0-255
  //------------------------------------------------------------------------------
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
  if (!rtc.begin()) {
    Serial.println("RTC found!");
    tft.drawString("RTC found!", xpos, ypos, 2);
    xpos = 0; ypos += 20;
  }
  
  //------------------------------------------------------------------------------
  // testAT24C32();              // тест
  // tft.drawString("AT24C32 test complete.", xpos, ypos, 2);
  // xpos = 0; ypos += 20;
  //==============================================================================
  // initKeypad();
  // initFreeFont();

  //==============================================================================
  // Serial.println("---------------ESP8266 <-> DS18B20 Temperature Sensor ----------------");

  // Инициализация библиотеки DallasTemperature
  // sensors.begin();
  // sensors.setWaitForConversion(false);    // false: функция вернет управление немедленно.
  // sensors.setCheckForConversion(false);   // Часто используется вместе с waitForConversion = false
  // sensors.setAutoSaveScratchPad(false);   // Флаг автоматического сохранения настроек в EEPROM датчика.
  // sensors.setResolution(12);

  // Поиск устройств на шине 1-Wire
  // numberOfDevices = sensors.getDeviceCount();
  // if(numberOfDevices > MAX_DEVICE) numberOfDevices = MAX_DEVICE;
  // data[0] = NUMBER_FONT[numberOfDevices]; // отображение числа датчиков на дисплее
  // Serial.print("Found ");
  // Serial.print(numberOfDevices, DEC);
  // Serial.println(" devices.");

  // if (numberOfDevices == 0) {
  //   Serial.println("No DS18B20 sensors found! Check wiring and pull-up resistor.");
  //   // Можно остановить выполнение, если датчики не найдены
  //   // while(true) delay(100);
  // } else {
  //   sensors.requestTemperatures(); // Отправляем команду на измерение
  //   Serial.println("Sensor addresses:");
  //   // Выводим адрес каждого найденного устройства
  //   for (uint8_t i = 0; i < numberOfDevices; i++) {
  //     if (sensors.getAddress(sensorAddress, i)) {
  //       Serial.print("  Sensor ");
  //       Serial.print(i);
  //       Serial.print(": ");
  //       printAddress(sensorAddress);
  //       Serial.println();
  //     } else {
  //       Serial.print("Could not get address for sensor ");
  //       Serial.println(i);
  //     }
  //   }
    // Устанавливаем разрешение для всех датчиков (9, 10, 11, or 12 бит)
    // 12 бит дает наибольшую точность, но и наибольшее время преобразования (~750ms)
    // sensors.setResolution(12); // Уже по умолчанию 12 бит при инициализации
//}
  //==================================================================================
  delay(5000);
  tft.fillScreen(TFT_BLACK);
  diagram(grafDispl[0], TFT_WHITE);
  diagram(grafDispl[1], TFT_WHITE);
}

void loop() {
  //-------------------------------------------------------------------------------
  analogWrite(ledPin, brightness);      // Устанавливаем яркость светодиода
  brightness = brightness + fadeAmount; // Изменяем яркость для следующего шага
  // Меняем направление изменения яркости, если достигнуты пределы
  if (brightness <= 0 || brightness >= 255) { // analogWriteRange(255) /  Для диапазона по умолчанию 0-1023:
    fadeAmount = -fadeAmount;
  }
  delay(30);                            // Небольшая задержка для плавности эффекта
  //========================================================================================================
  // loopKeypad();
  // loopFreeFont();
  //========================================================================================================
  long now = millis();
  if (now - lastMsg > 1000) {
    seconds++;
    lastMsg = now;
    pwTriac = UpdatePID(&pid[0],0);            // ПИД нагреватель
    //-----температура воздуха------
    dpv0 = (float)pid[0].pPart/500 + (float)(pid[0].output-5)/100;
    flT0+=dpv0;
    ds[0].pvT = flT0;
    int16_t pverr = set[0] - ds[0].pvT;
    //----температура среды------
      pverr = set[1] - ds[1].pvT;
      if(pverr>200) dpv1 = 6;
      else if(pverr>100) dpv1 = 5;
      else if(pverr>50) dpv1 = 3;
      else if(pverr>10) dpv1 = 2;
      else if(pverr>0) dpv1 = 1;
      else if(pverr<0) dpv1 = -1;
      ds[1].pvT+=dpv1;
  //================================================================
    displ_0();

  // if (numberOfDevices) {
  //   // Получаем температуру
  //   for (byte i = 0; i < numberOfDevices; i++)
  //   {
  //     float tempC = sensors.getTempCByIndex(i); // Температура в градусах Цельсия
  //     // Проверка на корректность чтения
  //     if (tempC == DEVICE_DISCONNECTED_C) { // DEVICE_DISCONNECTED_C обычно -127
  //       Serial.printf("Error%d= %.2f °C\n", i, tempC);
  //       errDevice[i]++;
  //     } else {
  //       if(i==0) t1 = tempC*10;
  //       Serial.printf("T%d= %.2f °C\n", i, tempC);
  //       errDevice[i] = 0;
  //     }
  //   }
  //   Serial.println();
  //   sensors.requestTemperatures(); // Отправляем команду на измерение
  // } else {
  //   Serial.println("No sensors to read from.");
  // }
    //-------------------------
    DateTime now = rtc.now();
    //-----------------------------------------------------------------------------
    // -- Пример 1: Управление выходами PCF8574 (как светодиодами) ---
    writePCF8574(now.second()%10);
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
      Serial.println("Pin P8 is LOW");
    } else {
      Serial.println("Pin P8 is HIGH");
    }
    */
    
  }
    //-----------------------------------------------------------------------------
}

// Функция для записи байта на PCF8574
byte writePCF8574(byte data) {
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(data);
  byte error = Wire.endTransmission();
  if (error == 0) {
    //Serial.print("Data written: 0b");
    //printBinary(data);
    //Serial.println();
  } else {
    Serial.print("Error writing to PCF8574. Error code: ");
    Serial.println(error);
  }
  return error;
}

// Функция для чтения байта с PCF8574
byte readPCF8574() {
  Wire.requestFrom(PCF8574_ADDRESS, 1); // Запросить 1 байт данных
  if (Wire.available()) {
    return Wire.read();
  } else {
    Serial.println("Error reading from PCF8574: No data available.");
    return 0xFF; // Возвращаем 0xFF в случае ошибки (можно выбрать другое значение)
  }
}

// Вспомогательная функция для печати байта в двоичном формате
void printBinary(byte inByte) {
  for (int b = 7; b >= 0; b--) {
    Serial.print(bitRead(inByte, b));
  }
}

// Вспомогательная функция для вывода адреса датчика
// void printAddress(DeviceAddress deviceAddress) {
//   for (uint8_t i = 0; i < 8; i++) {
//     if (deviceAddress[i] < 16) Serial.print("0");
//     Serial.print(deviceAddress[i], HEX);
//     if (i < 7) Serial.print(":");
//   }
// }

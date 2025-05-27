// #include <Arduino.h>
#include <SPI.h>
#include "ili9341.h"
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
int16_t t1 = 375, t2 = 302;

byte writePCF8574(byte data);
byte readPCF8574();
void testAT24C32();
// void printAddress(DeviceAddress deviceAddress);
RTC_DS3231 rtc;               // Создаем объект RTC для DS3231

// Создаем объекты TFT
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS); // Используем только CS
//XPT2046_Touchscreen ts(TOUCH_CS, TIRQ_PIN); // Если используете T_IRQ

void setup() {
  Serial.begin(115200);       // Инициализация последовательного порта для отладки
  //------------------------------------------------------------------------------
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

  //---------------------------------------  Инициализация DS3231 ----------------------------------------
  if (!rtc.begin()) {
    // Serial.println("Couldn't find RTC! Check wiring or I2C address.");
    // Serial.flush();       // гарантированно вывелось в монитор порта
    // while (1) delay(10);  // Остановка, если RTC не найден
  }
  Serial.println("RTC found!");
  //------------------------------------------------------------------------------
  testAT24C32();              // тест
  //==============================================================================
  initTFT();

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
  delay(2000);
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
  //-------------------------------------------------------------------------------
  // Проверяем, есть ли нажатие
  if (ts.touched()) touchTest();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    //===================
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
  //     // Serial.println("No sensors to read from.");
  //     if(t1>400) t1 = 375;
  //     else t1++;
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

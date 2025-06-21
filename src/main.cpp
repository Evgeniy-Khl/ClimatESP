#include "main.h"
#include "tftArcFill.h"
#include "display.h"
#include "touchKeypad.h"
#include "procedure.h"

// #include <OneWire.h>
// #include <DallasTemperature.h>
#include "AT24C32.h"
#include "my_settings.h"

PIDController pid[2];

// void printAddress(DeviceAddress deviceAddress);
RTC_DS3231 rtc;               // Создаем объект RTC для DS3231

// Создаем объекты TFT
TFT_eSPI tft = TFT_eSPI();    // Создаем экземпляр библиотеки

void setup() {
  Serial.begin(115200);       // Инициализация последовательного порта для отладки
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();
  //--------- инициализация SPIFFS -----------------------------------------
  if (!SPIFFS.begin()) {
      Serial.println("ERROR file system!");
      tft.setTextDatum(TC_DATUM);
      tft.setTextColor(TFT_RED, TFT_YELLOW);
      tft.drawString("ERROR file system!", tft.width()/2, tft.height()/2-20, 4);
      delay(10000);
  }
  //--------- инициализация Конфигурации --------------------------------------------
  initMyConfig();

  
}

void loop() {
  //-------------------------------------------------------------------------------
  analogWrite(LEDPIN, brightness);      // Устанавливаем яркость светодиода
  brightness = brightness + fadeAmount; // Изменяем яркость для следующего шага
  // Меняем направление изменения яркости, если достигнуты пределы
  if (brightness <= 0 || brightness >= 255) { // analogWriteRange(255) /  Для диапазона по умолчанию 0-1023:
    fadeAmount = -fadeAmount;
  }
  delay(30);                            // Небольшая задержка для плавности эффекта
  //========================================================================================================
  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);
  if(pressed && !newDispl){
    switch (displNum){
    case 0: 
      // tft.loadFont("Arial28"); // загрузка в память шрифта
      // Serial.println("main():Arial28");
      // tft.setTextDatum(TC_DATUM);
      displNum = 1; newDispl = true;
      menu_1();
      break;
    case 1: checkKeypad(MENU_1); break;
    case 2: checkKeypad(MENU_1); break;
    case 3: checkKeypad(MENU_2); break;
    case 4: checkKeypad(MENU_3); break;

    case 10: checkKeypad(15); break;
    }
    // if(displNum==0) displNum = 1;
    // newDispl = true;
    // if(++displNum>3) displNum = 0;  
  } 
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
    int16_t pverr = settings.sp_structs[0].spT - ds[0].pvT;
    //----температура среды------
      pverr = settings.sp_structs[1].spT - ds[1].pvT;
      if(pverr>200) dpv1 = 6;
      else if(pverr>100) dpv1 = 5;
      else if(pverr>50) dpv1 = 3;
      else if(pverr>10) dpv1 = 2;
      else if(pverr>0) dpv1 = 1;
      else if(pverr<0) dpv1 = -1;
      ds[1].pvT+=dpv1;
  //================================================================
    if(displNum == 0) mainDispl();

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

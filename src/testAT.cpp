#include <AT24C32.h>

void testAT24C32() {
  Serial.println("---------------ESP8266 <-> AT24C32 EEPROM Test---------------");

  uint16_t byteAddr = 0;
  uint16_t intAddr = 10;
  uint16_t floatAddr = 20;
  uint16_t stringAddr = 30;

  // --- Демонстрация записи ---
  Serial.print("\nWriting data to EEPROM. EEPROM_I2C_ADDRESS: 0x"); Serial.println(EEPROM_I2C_ADDRESS, HEX);

  // Запись одного байта
  uint8_t testByte = 0xA5;
  eepromWriteByte(byteAddr, testByte);
  Serial.print("Wrote byte 0x"); Serial.print(testByte, HEX); Serial.print(" to address "); Serial.println(byteAddr);

  // Запись целого числа (int - 4 байта на ESP8266)
  int testInt = 12345;
  eepromWriteBuffer(intAddr, (uint8_t*)&testInt, sizeof(testInt));
  Serial.print("Wrote int "); Serial.print(testInt); Serial.print(" to address "); Serial.println(intAddr);
  
  // Запись числа с плавающей точкой
  float testFloat = 3.14159f;
  eepromWriteFloat(floatAddr, testFloat);
  Serial.print("Wrote float "); Serial.print(testFloat, 5); Serial.print(" to address "); Serial.println(floatAddr);

  // Запись строки
  String testString = "Hello AT24C32!";
  eepromWriteString(stringAddr, testString);
  Serial.print("Wrote string '"); Serial.print(testString); Serial.print("' to address "); Serial.println(stringAddr);

  Serial.println("\nWrite operations complete.");
  Serial.println("------------------------------------");
  delay(1000); // Дадим время на осмысление

  // --- Демонстрация чтения ---
  Serial.println("Reading data from EEPROM...");
  Serial.println();
  // Чтение одного байта
  uint8_t readB = eepromReadByte(byteAddr);
  Serial.print("Read byte from address "); Serial.print(byteAddr); Serial.print(": 0x"); Serial.println(readB, HEX);

  // Чтение целого числа
  int readI;
  eepromReadBuffer(intAddr, (uint8_t*)&readI, sizeof(readI));
  Serial.print("Read int from address "); Serial.print(intAddr); Serial.print(": "); Serial.println(readI);

  // Чтение числа с плавающей точкой
  float readF = eepromReadFloat(floatAddr);
  Serial.print("Read float from address "); Serial.print(floatAddr); Serial.print(": "); Serial.println(readF, 5);

  // Чтение строки
  // Укажите максимальную ожидаемую длину строки. Она должна быть не больше, чем было записано.
  // Если строка короче, она будет корректно прочитана благодаря нулевому терминатору.
  String readS = eepromReadString(stringAddr, 30); // 30 - максимальная длина буфера для чтения
  Serial.print("Read string from address "); Serial.print(stringAddr); Serial.print(": '"); Serial.print(readS); Serial.println("'");
  
  Serial.println("\nRead operations complete.");
}
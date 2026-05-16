#include "main.h"
#include <AT24C32.h>

// AT24C32 имеет страницы по 32 байта. 128 страниц ВСЕГО 4 096 байт
// каждые 5 мин. по 4 байта за сутки 1 152 байт ровно 36 стр.

// ----- Функции для работы с EEPROM -----

/**
 * @brief Записывает один байт в EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @param data Байт для записи.
 */
void eepromWriteByte(uint16_t memoryAddress, uint8_t data) {
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));
  Wire.write((uint8_t)(memoryAddress & 0xFF));
  Wire.write(data);
  uint8_t status = Wire.endTransmission();
  if (status != 0) {
    MYDEBUG_PRINT("I2C Write Error for byte at addr "); MYDEBUG_PRINT(memoryAddress);
    MYDEBUG_PRINT(". Status: "); MYDEBUG_PRINTLN(status);
    // Коды ошибок: 0:success, 1:data too long, 2:NACK on address, 3:NACK on data, 4:other
  }
  delay(EEPROM_WRITE_DELAY);
}

/**
 * @brief Читает один байт из EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @return Прочитанный байт или 0 если произошла ошибка.
 */
uint8_t eepromReadByte(uint16_t memoryAddress) {
  uint8_t readData = 0;
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));   // Старший байт адреса
  Wire.write((uint8_t)(memoryAddress & 0xFF)); // Младший байт адреса
  Wire.endTransmission(); // Отправка адреса без STOP для "dummy write"

  Wire.requestFrom(EEPROM_I2C_ADDRESS, 1); // Запросить 1 байт
  if (Wire.available()) {
    readData = Wire.read();
  }
  return readData;
}

bool eepromWriteBuffer(uint16_t memoryAddress, const uint8_t* buffer, uint16_t length) {
  if (buffer == nullptr || length == 0) {
    return true; // Нет данных для записи, считаем операцию успешной.
  }

  uint16_t currentBufferIndex = 0;
  while (currentBufferIndex < length) {
    // Рассчитываем, сколько байт можно записать до конца текущей страницы EEPROM
    uint8_t bytesToWrite = EEPROM_PAGE_SIZE - (memoryAddress % EEPROM_PAGE_SIZE);

    // Убеждаемся, что не пытаемся записать больше, чем осталось в буфере
    uint16_t remainingBytes = length - currentBufferIndex;
    if (bytesToWrite > remainingBytes) {
      bytesToWrite = remainingBytes;
    }

    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((uint8_t)(memoryAddress >> 8));   // Старший байт адреса
    Wire.write((uint8_t)(memoryAddress & 0xFF)); // Младший байт адреса

    // Записываем порцию данных одним вызовом
    Wire.write(buffer + currentBufferIndex, bytesToWrite);

    // Завершаем передачу и проверяем на ошибки
    if (Wire.endTransmission() != 0) {
      // Serial.println("I2C Write Error!"); // Раскомментируйте для отладки
      return false; // Сообщаем о неудаче
    }
    
    // Ожидание завершения внутреннего цикла записи EEPROM
    delay(EEPROM_WRITE_DELAY);

    // Сдвигаем адреса для следующей итерации
    memoryAddress += bytesToWrite;
    currentBufferIndex += bytesToWrite;
  }
  
  return true; // Все данные успешно записаны
}

uint16_t eepromReadBuffer(uint16_t memoryAddress, uint8_t* buffer, uint16_t length) {
  if (buffer == nullptr || length == 0) {
    return 0;
  }

  // Сначала устанавливаем внутренний указатель адреса в EEPROM
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));
  Wire.write((uint8_t)(memoryAddress & 0xFF));
  if (Wire.endTransmission() != 0) {
    // Не удалось даже установить адрес для чтения
    return 0;
  }

  uint16_t bytesRead = 0;
  // Теперь читаем данные порциями, чтобы не переполнять внутренний буфер Wire
  // Для ESP8266 можно и больше, но 32 - универсальное и безопасное значение
  const uint8_t chunkSize = 32; 

  while (bytesRead < length) {
    uint16_t bytesToRequest = length - bytesRead;
    if (bytesToRequest > chunkSize) {
      bytesToRequest = chunkSize;
    }

    uint16_t received = Wire.requestFrom((uint8_t)EEPROM_I2C_ADDRESS, (uint8_t)bytesToRequest);

    // Если устройство вернуло 0 байт, значит, что-то пошло не так
    if (received == 0) {
      break; // Прерываем цикл, возвращаем то, что успели прочитать
    }

    for (uint16_t i = 0; i < received; i++) {
      buffer[bytesRead + i] = Wire.read();
    }
    
    bytesRead += received;
  }

  return bytesRead;
}

/**
 * @brief Очищает область памяти в AT24C32, используемую для хранения суточных данных.
 * Заполняет нулями 288 записей (t1 и t2).
 */
void clearEEPROM() {
  Serial.println("Начало очистки AT24C32...");
  
  // Проходим по всем 288 пятиминутным периодам
  for (int period = 0; period < DAILY_DATA_MAX_REC; ++period) {
    // Вычисляем адрес для текущего периода
    int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
    
    // Записываем 0 для t1, t2 и rh
    eepromWriteInt16(currentAddress, 0);
    eepromWriteInt16(currentAddress + 2, 0);
    eepromWriteInt16(currentAddress + 4, 0);

    // Выводим точку каждые 10 записей, чтобы показать, что процесс идет
    if (period % 10 == 0) Serial.print(".");
  }
  
  Serial.println("\nОчистка AT24C32 завершена.");
}

// ----- Функции для работы с разными типами данных -----
bool eepromWriteInt16(uint16_t address, int16_t value) {
  // Просто преобразуем значение в байты и передаем в нашу основную функцию
  return eepromWriteBuffer(address, (const uint8_t*)&value, sizeof(value));
}

bool eepromReadInt16(uint16_t address, int16_t& value) {
  uint16_t bytesRead = eepromReadBuffer(address, (uint8_t*)&value, sizeof(value));
  
  // Успешным считаем только то чтение, где количество байт совпало с ожидаемым
  return (bytesRead == sizeof(value));
}

bool eepromWriteFloat(uint16_t address, float value) {
  return eepromWriteBuffer(address, (const uint8_t*)&value, sizeof(value));
}

bool eepromReadFloat(uint16_t address, float& value) {
  uint16_t bytesRead = eepromReadBuffer(address, (uint8_t*)&value, sizeof(value));
  return (bytesRead == sizeof(value));
}

/* void eepromWriteString(uint16_t address, const String& str) {
  // Записываем длину строки первым байтом, затем саму строку (макс. длина 254 + 1 байт для null)
  // или просто строку до \0, если не хотим хранить длину явно.
  // Для простоты запишем строку как есть, включая нулевой терминатор.
  uint16_t len = str.length() + 1; // +1 для '\0'
  char charBuf[len];
  str.toCharArray(charBuf, len);
  eepromWriteBuffer(address, (uint8_t*)charBuf, len);
}

String eepromReadString(uint16_t address, uint16_t maxLength) {
  char charBuf[maxLength + 1]; // Буфер для строки + '\0'
  eepromReadBuffer(address, (uint8_t*)charBuf, maxLength);
  charBuf[maxLength] = '\0'; // Гарантируем нулевой терминатор

  // Найдем фактический конец строки, если он раньше maxLength
  for(uint16_t i=0; i<maxLength; ++i) {
    if (charBuf[i] == '\0') break;
  }
  return String(charBuf);
}
 */
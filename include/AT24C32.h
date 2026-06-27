#include "Arduino.h"
#include <Wire.h>

// I2C адрес AT24C32.
// Если A0, A1, A2 подключены к GND, адрес 0x50.
// Адрес может быть от 0x50 до 0x57 в зависимости от состояния пинов A0-A2.
#define EEPROM_I2C_ADDRESS 0x57
// Размер страницы для AT24C32 (для AT24C128 будет 64)
#define EEPROM_PAGE_SIZE   32 
// Задержка после операции записи в мс (время записи страницы для AT24C32 до 5мс)
#define EEPROM_WRITE_DELAY 5

/**
 * @brief Записывает один байт в EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @param data Байт для записи.
 */
void eepromWriteByte(uint16_t memoryAddress, uint8_t data);

/**
 * @brief Читает один байт из EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @return Прочитанный байт или 0 если произошла ошибка.
 */
uint8_t eepromReadByte(uint16_t memoryAddress);

/**
 * @brief Записывает массив байт (буфер) в I2C EEPROM с обработкой границ страниц.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на массив байт для записи.
 * @param length Количество байт для записи.
 * @return true, если вся операция записи прошла успешно, иначе false.
 */
bool eepromWriteBuffer(uint16_t memoryAddress, const uint8_t* buffer, uint16_t length);

/**
 * @brief Читает массив байт (буфер) из I2C EEPROM.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на буфер для сохранения прочитанных данных.
 * @param length Количество байт для чтения.
 * @return Количество фактически прочитанных байт. В случае ошибки может быть меньше, чем length.
 */
uint16_t eepromReadBuffer(uint16_t memoryAddress, uint8_t* buffer, uint16_t length);

/**
 * @brief Очищает область памяти в AT24C32, используемую для хранения суточных данных.
 * Заполняет нулями 288 записей (t1 и t2).
 * @param quiet Если true, очистка происходит без звуков и вывода на дисплей.
 */
void clearEEPROM(bool quiet = false);

/** @return true в случае успеха, иначе false. */
bool eepromWriteInt16(uint16_t address, int16_t value);

/** @return true и записывает значение в 'value' в случае успеха, иначе false. */
bool eepromReadInt16(uint16_t address, int16_t& value);

/** @return true в случае успеха, иначе false. */
bool eepromWriteFloat(uint16_t address, float value);

/** @return true и записывает значение в 'value' в случае успеха, иначе false. */
bool eepromReadFloat(uint16_t address, float& value);

void eepromWriteString(uint16_t address, const String& str);
String eepromReadString(uint16_t address, uint16_t maxLength);

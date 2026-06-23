// server.h
#ifndef SERVER_H
#define SERVER_H

#define SET1a     40      // (35)*64*0.25= 560 us (0.5 mS)  (0x32)   0 грд.
#define SET1b     85      // (90)*64*0.25= 1440 us (1.4 mS)  (0x5E)  90 грд.
#define MIN_R_M   0

// #include <ESPAsyncWebServer.h>
#include <ESP8266WebServer.h>

#define MYPORT_TX 13
#define MYPORT_RX 12
#define RAMPV_SIZE   33
#define EEPROM_SIZE  34
#define PROG_SIZE    64

struct Program {
  int16_t prT0;     // 2 байт Уставка температуры Сухой датчик
  uint8_t prT1;     // 1 байт Уставка температуры Влажный датчик
  uint8_t prRh;     // 1 байт Уставка HIH-5030/AM2301
  uint8_t prFlp;    // 1 байт текущее положение залонки
};

union Series {
  uint8_t receivedData[5]; // Массив для приема
  Program pr;
};
extern Series srs;

enum Interval { INTERVAL_1000 = 1000, INTERVAL_4000 = 4000 };
extern Interval interval;
extern ESP8266WebServer server;

// Функция для обработки параметров
void notFoundHandler();
void respondsEeprom();
uint8_t respondsProgram();
void acceptEeprom();
void acceptProgram();
void respondsValues();
byte calculateChecksum(byte* data, int length);
void OutStatusLed();      // для HTML страницы
void sendPageHeader(String title);  // Отправляет общую "шапку" HTML для всех страниц, включая CSS-стили.
void handleArchiveList();           // Генерирует HTML-страницу со списком всех архивных файлов (_graph.json).
void handleShowData();              // Генерирует страницу с таблицей, отправляя ВЕСЬ HTML по частям.
void handleGetGraph();              // Отдает сырой JSON файл для графика.
void handleGetCurrentGraph();       // Генерирует JSON для текущего графика из EEPROM.
void handleCurrentData();           // Генерирует таблицу с данными за ТЕКУЩИЙ день, читая их напрямую из AT24C32.
void handleViewLogs();              // Страница просмотра логов системы
void handleClearLogs();             // Сброс файлов логов
void handleLogs();                  // Отдает сырой лог файл в зависимости от параметра day
void streamFileChunked(File& file, const String& contentType); // Безопасная отдача файлов чанками

#endif /*SERVER_H*/

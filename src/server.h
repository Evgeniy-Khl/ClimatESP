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

struct Eeprom {
  int16_t spT[2];     // 4 байт Уставка температуры spT[0]->Сухой датчик; spT[1]->Влажный датчик 
  int8_t  spRH[2];    // 2 байт spRH[0]->ПОДСТРОЙКА HIH-5030/AM2301 spRH[1]->Уставка HIH-5030/AM2301 
  uint8_t ventMode;   // 1 байт 1-ОХЛАЖДЕНИЕ; 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ 
  uint8_t extMode;    // 1 байт 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ 2-ВСПОМОГАТЕЛЬНЫЙ НАГРЕВАТЕЛЬ
  uint8_t relayMode;  // 1 байт релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
  uint8_t checkDry;   // 1 байт задержка регулировки по влажному, пока не установиться по сухому      
  uint8_t rotate[2];  // 2 байт [0]-отключ.состояниe [1]-включ.состояниe
  uint8_t program;   // 1 байт работа по программе
  uint8_t alarm[2];   // 2 байт СИРЕНА
  uint8_t vent[2];    // 2 байт ВЕНТИЛЯЦИЯ 
  uint8_t extOn;      // 1 байт смещение для ВКЛ. вспомогательного канала
  uint8_t extOff;     // 1 байт смещение для ОТКЛ. вспомогательного канала
  uint8_t minRun;     // 1 байт импульсное управление помпой увлажнителя
  uint8_t maxRun;     // 1 байт импульсное управление помпой увлажнителя
  uint8_t period;     // 1 байт импульсное управление помпой увлажнителя
  uint8_t hysteresis; // 1 байт гистерезис канала увлажнения маска 0x03; разрешение использования HIH-5030 маска 0x40; AM2301 маска 0x80;    
  uint8_t air[2];     // 2 байт таймер проветривания air[0]-пауза; air[1]-работа; если air[1]=0-ОТКЛЮЧЕНО
  uint8_t flpClose;   // 1 байт положение залонка ЗАКРЫТА
  uint8_t flpOpen;    // 1 байт положение залонка ОТКРЫТА
  uint8_t flpNow;     // 1 байт текущее положение залонки
  uint8_t pkoff[2];   // 2 байт ind=29;ind=30 пропорциональный коэфф.
  uint8_t ikoff[2];   // 2 байт ind=31;ind=32 интегральный коэфф.
  uint8_t identif;    // 1 байт сетевой номер прибора
  uint8_t status;     // 1 байт не используется
};// --------- 34 bytes --------

union Setting{
  uint8_t receivedData[EEPROM_SIZE]; // Массив для приема
  Eeprom sp;
};

extern Setting set;

struct Rampv {
    uint8_t model;       // 1 байт ind=0  модель прибора
    uint8_t node;        // 1 байт ind=1  сетевой номер прибора
    int16_t t[3];        // 6 байт ind=2-ind=9   значения датчиков температуры
    uint8_t rH;          // 1 байт ind=8  значение датчика относительной влажности
    uint8_t pvOut;       // 1 байт ind=9  активные выходы реле
    uint8_t timer;       // 1 байт ind=10 значение таймера до начала поворота лотков
    uint8_t fan;         // 1 байт ind=11 скорость вращения тихоходного вентилятора
    uint8_t flap;        // 1 байт ind=12 положение заслонки 
    uint8_t power;       // 1 байт ind=13 мощность подаваемая на тены
    uint8_t fuses;       // 1 байт ind=14 короткие замыкания 
    uint8_t errors;      // 1 байт ind=15 ошибки
    uint8_t warning;     // 1 байт ind=16 предупреждения
    uint8_t currDay;     // 1 байт ind=17 дней инкубации
    uint8_t currHour;    // 1 байт ind=18 часов инкубации
    uint8_t currMin;     // 1 байт ind=19 минут инкубации
    int16_t spT[2];     // 4 байт Уставка температуры spT[0]->Сухой датчик; spT[1]->Влажный датчик 
    int8_t  spRH[2];    // 2 байт spRH[0]->ПОДСТРОЙКА HIH-5030/AM2301 spRH[1]->Уставка HIH-5030/AM2301     
    uint8_t ventMode;   // 1 байт 1-ОХЛАЖДЕНИЕ; 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ 
    uint8_t extMode;    // 1 байт 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ 2-ВСПОМОГАТЕЛЬНЫЙ НАГРЕВАТЕЛЬ
    uint8_t relayMode;  // 1 байт релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
    uint8_t checkDry;   // 1 байт задержка регулировки по влажному, пока не установиться по сухому      
    uint8_t rotate[2];  // 2 байт [0]-отключ.состояниe [1]-включ.состояниe
    uint8_t program;   // 1 байт работа по программе
};// ------- 20+13 bytes ------------
union Upv{
  uint8_t receivedData[RAMPV_SIZE]; // Массив для приема
  Rampv pv;
};

enum Interval { INTERVAL_1000 = 1000, INTERVAL_4000 = 4000 };
extern Interval interval;
extern Upv upv;
extern ESP8266WebServer server;

// Функция для обработки параметров
void notFoundHandler();
void respondsEeprom();
void respondsProgram();
void acceptEeprom();
void acceptProgram();
void respondsValues();
String getFloat(float val, uint8_t brackets);
byte calculateChecksum(byte* data, int length);
void OutStatus();

#endif //SERVER_H
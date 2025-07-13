#ifndef __MAIN_H
#define __MAIN_H

#define LED_DISPLAY

#include <Arduino.h>
#include <MyTelegramBot.h>  // Universal Telegram Bot Library written by Brian Lough: 
#include <WiFiManager.h>    //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#define FlashFS LittleFS
#include <SPI.h>
#include <Wire.h>     // Библиотека для I2C связи
#include <RTClib.h>   // Библиотека для работы с RTC DS3231
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SoftwarePWMBit.h" // Подключаем наш новый класс
#include "AT24C32.h"
#include "server.h"
#include "telegram.h"

#include "procedure.h"
#ifdef LED_DISPLAY
  #include "display_led.h"
  #include "keypad.h"
#else
  #include "display_tft.h"
#endif
#include "sensors.h"

#define DEBUG

#ifdef DEBUG
  // Вариативные макросы, принимающие любое количество аргументов
  #define DEBUG_SPRINTF(...)  sprintf(__VA_ARGS__)
  #define DEBUG_PRINT(...)   Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  // "Пустышки" остаются такими же
  #define DEBUG_SPRINTF(...)
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif
// --- Конец блока макросов ---

#define LEDPIN 2
#define ONE_WIRE_BUS_PIN LEDPIN   // используется номер GPIO2
#define MAX_DEVICE 4              // ограничение количества датчиков
#define START_MARKER 0xDD	// Начало блока = 221

#define READDEFAULT 0
#define READEEPROM  1
#define SAVEEEPROM  2
#define READPROG    3
#define SAVEPROG    4
#define GET_VALUES	0x40    // 64
#define GET_EEPROM	0x41    // 65
#define SET_EEPROM	0x42    // 66
#define GET_PROG1	  0x50    // 80
#define GET_PROG2	  0x51    // 81
#define GET_PROG3	  0x52    // 82
#define GET_PROG4	  0x53    // 83

#define SET_PROG1	  0x30    // 48
#define SET_PROG2	  0x31    // 49
#define SET_PROG3	  0x32    // 50
#define SET_PROG4	  0x33    // 51

typedef struct {
  int16_t pvT;              // текущее значение
  int16_t pvErr;            // текущая ошибка
  int16_t previousValue;    // предыдущее значение
  uint8_t errDevice;        // нет ответа датчика
  uint8_t deviation;        // отклонение от заданного значения
  uint16_t duration;        // длительность зависания
} Ds;
extern Ds ds[];

// Для предотвращения выравнивания полей компилятором, что может нарушить карту памяти.
// В данном случае все поля одного типа, и проблема маловероятна, но это хорошая практика.
#pragma pack(push, 1)
struct Sp{
    int16_t spT; 	      // Уставка температуры
    int16_t spRH;	      // Уставка относительной влажности (sp[0].spRH->ПОДСТРОЙКА HIH)
    int16_t alarm;      // дельта 5 = 0.5 гр.C
    int16_t coolOn;     // включение охлаждения
    int16_t coolOff;    // выключение охлаждения
    int16_t timer;      // длительность [0]-отключ.состояниe [1]-включ.состояниe
    int16_t aeration;   // [0]-ПАУЗА ПРОВЕТРИВАНИЯ (минут); [1]-ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
    int16_t auxiliary;  // [0]-включение форсированного; [1]-выключение форсированного
    int16_t state;      // [0]-заслонка текущее; [1]-программа текущая
    int16_t flapLimit;  // [0]-закрыта; [1]-открыта
    int16_t pulse;      // [0]-MIN; [1]-Период импульсов Sec.
    int16_t mode;       // [0]-релейный 0-НЕТ; 1->по кан.0 2->по кан.1 3->по кан.0&1; 4-импульсное по кан.1; [1]-задержка регулировки по влажному
    int16_t extendMode; // [0]-0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; [1]-1-ОХЛАЖДЕНИЕ; 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ
    int16_t Kp;         // Пропорциональный
    int16_t Ki;         // Интегральный
};                      //---- 30 уставок ----
#pragma pack(pop)

// Определяем union
union SpUnion {
    // Представление 1: Как массив из двух структур
    Sp sp_structs[2];
    // Представление 2: Как линейный массив из 32-х 16-битных значений
    int16_t flat_array[32]; // 16 полей * 2 структуры = 32
};

struct Bitfield {
    unsigned a0: 1;
    unsigned a1: 1;
    unsigned a2: 1;
    unsigned a3: 1;
    unsigned a4: 1;
    unsigned a5: 1;
    unsigned a6: 1;
    unsigned a7: 1;
};
 
union Byte {
    unsigned char value;
    struct Bitfield bitfield;
};

extern union Byte portOut;
extern union Byte errorsFlag;
extern union Byte portFlag;

#define HEATER  portOut.bitfield.a0  // НАГРЕВАТЕЛЬ
#define HUMIDI	portOut.bitfield.a1  // УВЛАЖНИТЕЛЬ
#define TURN		portOut.bitfield.a2  // Поворот лотков
#define EXTRA1	portOut.bitfield.a3  // Заслонка/вентилятор охлаждения
#define EXTRA2	portOut.bitfield.a4  // Вспомогательный нагреватель
#define EXTRA3 	portOut.bitfield.a5  // Авария

#define ERROR1    errorsFlag.bitfield.a0  // ОШИБКА ДАТЧИКА 0  --- потерян; 66,0-завис [E01]
#define ERROR2	  errorsFlag.bitfield.a1  // ОШИБКА ДАТЧИКА 1  --- потерян; 66,0-завис [E02]
#define ERROR4	  errorsFlag.bitfield.a2  // ОТКЛОНЕНИЕ КАНАЛ 0 [E04]
#define ERROR8	  errorsFlag.bitfield.a3  // ОТКЛОНЕНИЕ КАНАЛ 1 [E08]
#define ERROR10	  errorsFlag.bitfield.a4  // отказ одного из двух датчиков температуры
#define ERROR20	  errorsFlag.bitfield.a5  // отказ вспомогательного датчика температуры
#define OVERHEAT  errorsFlag.bitfield.a6  // перегрев симистора
#define FROZE	    errorsFlag.bitfield.a7  // завис датчик.

#define REACHED0    portFlag.bitfield.a0  // pvT[0]-ДОСТИГ spT[0]
#define REACHED1    portFlag.bitfield.a1  // pvT[1]-ДОСТИГ spT[1]
#define TURNSECOND  portFlag.bitfield.a2  // устанавливается в 1 если отсчет в секундах
#define NON2    portFlag.bitfield.a3  // не используется
#define HIH5030	    portFlag.bitfield.a4  // exist HIH5030 flag
#define AM2301	    portFlag.bitfield.a5  // exist AM2301 flag
#define COOLING     portFlag.bitfield.a6  // охлаждение
#define AERATION    portFlag.bitfield.a7  // проветривание

#define ON  1
#define OFF 0
#define TRIACON     999
#define DISPLAYOFF  300

#ifdef LED_DISPLAY
  #define BEEP_PIN        0
  #define PWMOUT_PIN      15
  #define RESETDISPLAY    20  // 20/2=10 Sec.
  #define MINWAIT         100
  #define WAITCHECKKEYPAD 1000	// mSec. максимальная пауза перед реакцией на кнопку
  extern TM1638 module;
  extern uint8_t data[8];
#else

#endif
//******************************************************** */
#define SPT_0       350 // завдання №1 35,0 °C
#define SPRH_0      0   // завдання (ПОДСТРОЙКА HIH) = 0
#define ALARM_0     5   // аварійне відхилення нагрів №1 0,5 °C
#define COOLON_0    3   // охолоджувач увімкнути №1 0,3 °C
#define COOLOFF_0   1   // охолоджувач вимкнути №1 0,1 °C
#define TIMER_0     60  // лотки увім. = 60 хв.
#define AERATION_0  10  // провітр.пауза ПАУЗА ПРОВЕТРИВАНИЯ (минут)
#define AUXILIARY_0 5   // допоміж. увім. = 5 (0,5 °C)
#define STATE_0     0   // заслінка полож. = 0 - закрита; 100 - відкрита
#define FLPCLOSE    0   // заслінка закр.
#define PULSE_0     100 // імпульс мінім. = 100 (1,0 сек.); 1 = 0,01 sec. 10 = 0,1 sec.; 1000 = 10,0sec.
#define MODE_0      0   // затрим. зволож. -> 1
#define EXTMODE_0   0   // аварійн. режим = 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
#define KP_0_1      80  // пропорц. 2 (Kp/4) = 80       [20*4]
#define KI_0_1      400 // ітеграл. 2 (Ki/10000) = 400  [20/500*10000]

#define SPT_1       300 // завдання у грд.Цесія №2 30,0 °C
#define SPRH_1      670 // завдання у відсотках №2 65%
#define ALARM_1     15  // аварійне відхилення №2 1,5 °C
#define COOLON_1    10  // осушувач увімкнути №2 = 10 (1,0 °C)
#define COOLOFF_1   5   // осушувач вимкнути №2 = 5 (0,5 °C)
#define TIMER_1     0   // лотки вимкн. = 0
#define AERATION_1  0   // провітр. робота ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд) = 0
#define AUXILIARY_1 2   // допоміж. вимкн. = 2 (0,2 °C)
#define STATE_1     0   // задана програма = 0
#define FLPOPEN     100 // заслінка закр.
#define PULSE_1     15  // імпульс період = 15 (sec.) MAX = 60 sec.
#define MODE_1      0   // режим реле = 0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]; 4-импульс
#define EXTMODE_1   0   // ???????????

extern char botToken[50];
extern char chatID [15];
extern MyTelegramBot bot;
extern bool shouldSaveConfig;
//-------------
extern uint8_t earlyMode, mode, tmrResetMode, quarter, errors, seconds;
extern int tableData[32][4], tmrTelegramOff;
extern uint16_t begHeapSize, previousHeapSize;
extern long lastSendTime, allTime; 
extern Interval interval;
//-------------

extern RTC_DS3231 rtc;
extern SpUnion settings;
extern DallasTemperature sensors;

extern bool newDispl;
extern long counterWait, counter10, counter1s;
extern uint8_t 
        numberOfDevices,    // число найденых датчиков
        displNum,           // вариант дисплея
        resetDispl,         // время ожидания до возврата главного диплея
        numSetup,           // пунк выбора установки
        halfSecond,         // счетчик полу-секунд
        pctHeater,          // значение мощности нагревателя
        pctHimidifier,      // значение мощности увлажнителя
        pvFlap,             // текущее положение заслонки
        beepOn,             // время звучания бипера
        disableBeep,        // время запрета включения аварийной сигнализации
        lastKey,            // последняя нажатая кнопка
        keys,               // текущая кнопка
        keyCount,           // время удержания последней кнопки
        countSeconds;       // счетчик секунд

extern int16_t 
        pvAeration,         // текущее время проветривания
        pvVenting,          // ? текущее время проветривания
        editBuff;           // временное хранилище редактируемой установки

extern uint16_t 
          pvVadcRH,         // значение АЦП
          pvRH,             // текущая относительная влажность
          heaterValue,      // расчетное значение ШИМ сигнала нагревателя
          humidiValue,      // расчетное значение ШИМ сигнала увлажнителя
          pvTimer,          // текущее значение таймера
          pvPulse,          // расчетное значение длетельности работы помпы
          pvPeriod,         // расчетное значение периода работы помпы
          waitCheckKeyPad;

extern const uint8_t tabRH[];
extern uint8_t dataLed[6];

byte writePCF8574(byte data);
byte readPCF8574();
void testAT24C32();
void initWiFiManag(void);

#endif /* __MAIN_H */


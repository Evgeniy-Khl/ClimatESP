#ifndef PROGRAMM_H
#define PROGRAMM_H
#include "main.h"

struct TableForOneDay{
    int16_t spT0; 	    // 0-999 Уставка температуры T0
    int16_t spT1; 	    // 0-999 Уставка температуры T1
    int16_t spRH;	      // 0-100 Уставка относительной влажности
    int16_t timer0;     // 0-240 Длительность отключ.состояниe
    int16_t timer1;     // 0-240 Длительность [1]-включ.состояниe
    int16_t aeration0;  // 0-240 ПАУЗА ПРОВЕТРИВАНИЯ (минут)
    int16_t aeration1;  // 0-240 ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
    int16_t flap;       // 0-100 Заслонка текущее положение
};

union TableBuff {
    uint8_t buffer[16];
    struct TableForOneDay spDay;
};

extern TableBuff unBuf;

uint16_t eepromMemoryAddressForDay(uint8_t prg, uint8_t day);
byte eepromWrBuff(uint16_t memoryAddress, const uint8_t* buffer, uint8_t length);
void eepromRdBuff(uint16_t memoryAddress, uint8_t* buffer, uint8_t length);
void prepareTable(uint8_t prg, uint8_t day, uint8_t amountday, int16_t t0, int16_t t1, 
  int16_t rh, int16_t timer, int16_t aer0, int16_t aer1, int16_t fl);
void prepareProg1();
void prepareProg2();
void prepareProg3();
void prepareProg4();
void testProgs();

#endif /* PROGRAMM_H */
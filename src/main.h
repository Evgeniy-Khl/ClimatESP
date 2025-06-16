#ifndef __MAIN_H
#define __MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "procedure.h"

typedef struct {
  int16_t pvT;
  uint8_t err;
} Ds;

extern Ds ds[];

struct Sp{
    int16_t spT; 	    // Уставка температуры
    int16_t spRH;	    // Уставка относительной влажности (sp[0].spRH->ПОДСТРОЙКА HIH)
    int16_t timer;      // длительность [0]-отключ.состояниe [1]-включ.состояниe
    int16_t alarm;      // дельта 5 = 0.5 гр.C
    int16_t coolOn;     // включение охлаждения
    int16_t coolOff;    // выключение охлаждения
    int16_t aeration;   // [0]-ПАУЗА ПРОВЕТРИВАНИЯ (минут); [1]-ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
    int16_t flapLimit;  // [0]-закрыта; [1]-открыта
    int16_t state;      // [0]-заслонка текущее; [1]-программа текущая
    int16_t service;    // [0]-включение форсированного; [1]-выключение форсированного
    int16_t pulse;      // [0]-MIN; [1]-MAX
    int16_t mode;       // [0]-релейный 0-НЕТ; 1->по кан.0 2->по кан.1 3->по кан.0&1; 4-импульсное по кан.1; [1]-задержка регулировки по влажному
    int16_t extendMode; // [0]-0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; [1]-период импульсов
    int16_t Kp;         // Пропорциональный
    int16_t Ki;         // Интегральный
    int16_t Kd;         // Дифференциальный
};

typedef struct
{
  uint16_t xpos; 
  uint16_t ypos; 
  uint8_t radius; 
  int16_t value; 
  int16_t sp;
} GrafDispl;

extern char displStr[];
extern bool newDispl;
extern uint8_t seconds, displNum, pwTriac;
extern uint16_t xpos, ypos, txt_height, t_x, t_y;
extern Sp sp[];

#endif /* __MAIN_H */
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <main.h>
#include "tftArcFill.h"

// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 130
#define KEY_W 55 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 5 // X and Y gap
#define KEY_SPACING_Y 5
#define KEY_TEXTSIZE 1   // Font size multiplier

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 318
#define DISP_H 100
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

#define SET1 "1-завдання нагрівача"
#define SET2 "2-завдання зволожув."
#define SET3 "3-лотки вгору"
#define SET4 "4-лотки униз"
#define SET5 "5-авар. відхилення Т1"
#define SET6 "6-авар. відхилення Т2"
#define SET7 "7-охолодж. увімкнено"
#define SET8 "8-охолодж. вимкнено"
#define SET9 "9-провітр. увімкнено"
#define SET10 "10-провітр. вимкнено"
#define SET11 "11-положення заслінки"
#define SET12 "12-програма інкубації"
#define SET13 "13-заслінка закрита"
#define SET14 "14-заслінка відкрита"
#define SET15 "15-мінім. імпульс"
#define SET16 "16-максим. імпульс"
#define SET17 "17-період імпульсів"
#define SET18 "18-аварійний режим"
#define SET19 "19-режим роботи реле"
#define SET20 "20-пропорц. коефіціент"
#define SET21 "21-ітеграл. коефіціент"
#define SET22 "22-дифер. коефіціент"

extern TFT_eSPI_Button key[];
extern const char* keyLabel[15];
extern uint16_t keyColor[15];

void display(void);
void displ_0(void);
void displ_1(void);
void displ_2(void);
void displ_3(void);
void drawKeypad(const char* keyLabel[], uint16_t keyColor[]);


uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);
void strUpdate(const char* txt, uint16_t* xpos, uint16_t ypos, uint16_t* txt_h);

#endif /* __DISPLAY_H */
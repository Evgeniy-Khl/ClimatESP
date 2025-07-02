#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <main.h>
#include "TM1638.h"

void ledDispl(void);
void menu_1(void);
void menu_2(void);
void menu_3(void);
void menu_4(void);
void calcDisplay(const char* txt);
void drawKeypad(const char* keyLabel[], uint16_t keyColor[]);
void drawKeypad_longName_7(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col);
void drawKeypad_longName_12(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col);
uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);
void initLedConfig(void);

#endif /* __DISPLAY_H */
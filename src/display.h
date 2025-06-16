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

#define SET1 "#1-завдання нагрівача"
#define SET2 "#2-завдання зволожувача"
#define SET3 "#1-завдання нагрівача"
#define SET4 "#1-завдання нагрівача"
#define SET5 "#1-завдання нагрівача"
#define SET6 "#1-завдання нагрівача"
#define SET7 "#1-завдання нагрівача"
#define SET8 "#1-завдання нагрівача"
#define SET9 "#1-завдання нагрівача"
#define SET10 "#1-завдання нагрівача"
#define SET11 "#1-завдання нагрівача"
#define SET12 "#1-завдання нагрівача"
#define SET13 "#1-завдання нагрівача"
#define SET14 "#1-завдання нагрівача"
#define SET15 "#1-завдання нагрівача"
#define SET16 "#1-завдання нагрівача"
#define SET17 "#1-завдання нагрівача"
#define SET18 "#1-завдання нагрівача"
#define SET19 "#1-завдання нагрівача"
#define SET20 "#1-завдання нагрівача"

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
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <main.h>

// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 130
#define KEY_W 55 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 5 // X and Y gap
#define KEY_SPACING_Y 5
#define KEY_TEXTSIZE 1   // Font size multiplier

void display(void);
void displ_0(void);
void displ_1(void);
void displ_2(void);
void drawKeypad(const char* keyLabel[], uint16_t keyColor[]);


uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);
void strUpdate(const char* txt, uint16_t* xpos, uint16_t ypos, uint16_t* txt_h);

#endif /* __DISPLAY_H */
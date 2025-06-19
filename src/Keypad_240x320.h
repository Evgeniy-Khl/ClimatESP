#include "FS.h"
#include <TFT_eSPI.h>



// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 86
#define KEY_W 55 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 5 // X and Y gap
#define KEY_SPACING_Y 5
#define KEY_TEXTSIZE 1   // Font size multiplier





void drawKeypad();
void touch_calibrate();
void status(const char *msg);
void initKeypad(void);
void loopKeypad(void);
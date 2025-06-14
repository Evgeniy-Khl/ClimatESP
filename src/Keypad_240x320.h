#include "FS.h"
#include <TFT_eSPI.h>

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 86
#define KEY_W 55 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 5 // X and Y gap
#define KEY_SPACING_Y 5
#define KEY_TEXTSIZE 1   // Font size multiplier

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing it and character index
#define NUM_LEN 12


// We have a status line for messages
#define STATUS_X 120 // Centred on this
#define STATUS_Y 65

void drawKeypad();
void touch_calibrate();
void status(const char *msg);
void initKeypad(void);
void loopKeypad(void);
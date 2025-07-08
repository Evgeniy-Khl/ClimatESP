#ifndef _KEYPAD_H
#define _KEYPAD_H
#include "main.h"

#define KEY_1 1
#define KEY_2 16
#define KEY_3 2
#define KEY_4 32
#define KEY_5 4
#define KEY_6 64
#define KEY_7 8
#define KEY_7_1 8+1
#define KEY_7_2 8+16
#define KEY_7_3 8+2
#define KEY_7_4 8+32
#define KEY_7_5 8+4
#define KEY_7_6 8+64
#define KEY_7_8 8+128
#define KEY_8 128

void checkkey(uint8_t key);
void saveset(void);

#endif /*_KEYPAD_H*/

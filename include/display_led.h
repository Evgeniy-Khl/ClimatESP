#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <main.h>
#include "TM1638.h"

#define ERRORS	        2
#define DAY	            3
#define COOL	        4
#define COMMA	        1
#define ENDCOMMA	    3
#define NOCOMMA	        0
#define BL              0x00     // пусто
#define TOP             0x01     // -
#define DEF             0x40     // -
#define BOT             0x08     // -
#define TT              0x78     // t
#define EE              0x79     // E
#define FF              0x71     // F
#define RR              0x50     // r
#define DD              0x5e     // d
#define NN              0x54     // n
#define hh              0x74     // h
#define HH              0x76     // H
#define OO              0x5C     // o
#define CC              0x39     // С
#define PE              0x37     // П
#define GE              0x31     // Г
#define YY              0x6E     // У
#define PP              0x73     // P
#define GR              0x63     // o
#define UU              0x62     // u

void ledDispl(unsigned char mode);
void displ_top(signed int val, unsigned char comma);
void displ_bot(signed int val, unsigned char comma);
void clr_top(void);
void clr_bot(void);
void displ_67(signed int val, unsigned char mode);
void display_setup(void);
void displErrors(void);

#endif /* __DISPLAY_H */
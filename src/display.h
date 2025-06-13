#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <main.h>


void displ_0(void);
uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);
void strUpdate(const char* txt, uint16_t* xpos, uint16_t ypos, uint16_t* txt_h);

#endif /* __DISPLAY_H */
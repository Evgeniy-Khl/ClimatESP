#include "main.h"
#include "display.h"

//--------- ОСНОВНОЙ ЭКРАН ----------------------
void mainDispl(void){
  data[0] = NUMBER_FONT[ds[0].pvT/100];
  data[1] = NUMBER_FONT[(ds[0].pvT%100)/10] | 0b10000000;
  data[2] = NUMBER_FONT[ds[0].pvT%10];
  data[3] = NUMBER_FONT[ds[1].pvT/100];
  data[4] = NUMBER_FONT[(ds[1].pvT%100)/10] | 0b10000000;
  data[5] = NUMBER_FONT[ds[1].pvT%10];

  data[6] = NUMBER_FONT[seconds/10];
  data[7] = NUMBER_FONT[seconds%10];

  module.setDisplay(data, 8);
}

void menu_1(){
  
}

void menu_2(){
  
}

void menu_3(){
  
}

void menu_4(){
  
}

void calcDisplay(const char* txt){
  
}

void drawKeypad_longName_7(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  
}

void drawKeypad_longName_12(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  
}

void drawKeypad(const char* keyLabel[], uint16_t keyColor[]){
  
}

uint16_t lampUpdate(uint16_t xpos, uint16_t ypos){
   return false; 
}

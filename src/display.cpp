#include "main.h"
#include "display.h"
#include "tftArcFill.h"

TFT_eSPI_Button key[15];

/* const char* txt1_for_display_3[15] = {
"1-завдання нагрівача", "2-завдання зволожув.", "3-авар. відхилення Т1", "4-авар. відхилення Т2", "5-охолодж. увімкнено",
"6-охолодж. вимкнено", "7-лотки вгору", "8-лотки униз", "9-провітр. увімкнено", "10-провітр. вимкнено", "11-положення заслінки",
"12-програма інкубації", "", "", ""
};
const char* txt2_for_display_3[15] = {
"13-заслінка закрита", "14-заслінка відкрита", "15-мінім. імпульс","16-максим. імпульс",
"17-період імпульсів", "18-аварійний режим", "19-режим роботи реле", "20-пропорц. коефіціент",
"21-ітеграл. коефіціент", "22-дифер. коефіціент", "", "", "", "", ""
}; */

// Набор подписей display_1
const char* labelsMenu1[MENU_1] = {
  "завдання нагрівача", 
  "завдання зволожувача", 
  "аварійне відхилення", 
  "охолодж. увімкнути", 
  "охолодж. вимкнути", 
  "далі >"
};
// const char* labelsMenu1[15] = {
//   "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "", ">", "0"
// };
uint16_t colorsMenu1[MENU_1] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_YELLOW
};

// Набор подписей display_2
const char* labels_for_display_2[15] = {
  "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "", "", "", "<", "0"
};
uint16_t color_for_display_2[15] = {
  TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN,
                          TFT_RED, TFT_RED, TFT_BLUE, TFT_BLUE, 
                          TFT_CYAN, TFT_CYAN, TFT_BLACK, TFT_BLACK,
                          TFT_BLACK, TFT_YELLOW, TFT_WHITE

};

// Набор подписей display_3
const char* labels_for_display_3[15] = {
  "+1","+5","+10","+50","+100","-1","-5","-10","-50","-100","","","","X","Ok"
};
uint16_t color_for_display_3[15] = {
                          TFT_CYAN, TFT_CYAN, TFT_CYAN, TFT_CYAN,
                          TFT_CYAN, TFT_CYAN, TFT_CYAN, TFT_CYAN, 
                          TFT_CYAN, TFT_CYAN, TFT_BLACK, TFT_BLACK,
                          TFT_BLACK, TFT_RED, TFT_GREEN

};
//--------- ОСНОВНОЙ ЭКРАН ----------------------
void displ_0(void){
  uint16_t h;
  if(newDispl){
    tft.fillScreen(TFT_BLACK);
  }
//-----------
  if(grafDispl[0].value != ds[0].pvT || newDispl) {
    grafDispl[0].value = ds[0].pvT;
    diagram(grafDispl[0], TFT_WHITE);
  }
  if(grafDispl[1].value != ds[1].pvT || newDispl) {
    grafDispl[1].value = ds[1].pvT;
    diagram(grafDispl[1], TFT_WHITE);
  }
  newDispl = false;
//-----------
  h = lampUpdate(20, 130);
//-----------
  tft.setTextPadding(310);
  xpos = 5; ypos = h+8;
  tft.drawRect(xpos-5, ypos-4, 319, 70, TFT_WHITE);
  tft.loadFont("Arial20"); // загрузка в память шрифта
  tft.setTextDatum(TL_DATUM);
  h = tft.fontHeight();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(displStr,"РЕЖИМ: Р=%g  І=%g", pid[0].pPart, pid[0].iPart);
  // w = tft.textWidth("РЕЖИМ:");
  // tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  // tft.fillRect(xpos + w, ypos, tft.width() - w, h, TFT_BLACK);

  ypos += (h+3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  sprintf(displStr,"ПОВОРОТ: %3d сек.",seconds);
  // w = tft.textWidth("ПОВОРОТ:");
  // tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  // tft.fillRect(xpos + w, ypos, tft.width() - w, h, TFT_BLACK);

  ypos += (h+3);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprintf(displStr,"ІНКУБАЦІЯ: %3d сек.",seconds);
  // w = tft.textWidth("ІНКУБАЦІЯ:");
  // tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  // tft.fillRect(xpos + w, ypos, tft.width() - w, h, TFT_BLACK);
  tft.unloadFont(); // выгрузка шрифта из памяти
}

void displ_1(){
  if(newDispl){
    for (int i = 0; i < MENU_1; i++) {
      keyLabel[i] = labelsMenu1[i];
      keyColor[i] = colorsMenu1[i];
    }
    tft.fillScreen(TFT_BLACK);
    // tft.loadFont("Arial28"); // загрузка в память шрифта
    // tft.setTextDatum(TC_DATUM);
    tft.drawString("канал 1", 160, 5);
    drawKeypad_longName(keyLabel, keyColor, MENU_1, 1);
    // tft.unloadFont(); // выгрузка шрифта из памяти

    newDispl = false;
  }
}
/* void displ_1(void){
  // Create 15 keys for the keypad
  if(newDispl){
    for (int i = 0; i < 15; i++) {
      keyLabel[i] = labelsMenu1[i];
      keyColor[i] = colorsMenu1[i];
    }
    tft.fillScreen(TFT_BLACK);
    xpos = 5; ypos = 0;
    tft.loadFont("Calibri14"); // загрузка в память шрифта
    uint16_t h = tft.fontHeight();
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(xpos, ypos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.drawString(txt1_for_display_3[0], xpos, ypos); tft.drawString(txt1_for_display_3[1], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt1_for_display_3[2], xpos, ypos); tft.drawString(txt1_for_display_3[3], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt1_for_display_3[4], xpos, ypos); tft.drawString(txt1_for_display_3[5], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt1_for_display_3[6], xpos, ypos); tft.drawString(txt1_for_display_3[7], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt1_for_display_3[8], xpos, ypos); tft.drawString(txt1_for_display_3[9], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt1_for_display_3[10], xpos, ypos); tft.drawString(txt1_for_display_3[11], xpos+170, ypos);
    ypos += h;
    tft.setTextColor(TFT_YELLOW, TFT_BLACK, true);
    tft.drawString("#30-ТЕХНІЧНІ ПАРАМЕТРИ ДЛЯ ФАХІВЦІВ!", xpos, ypos);
    ypos += h;
    tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
    tft.drawString("#99-ПОВЕРНЕННЯ ДО ГОЛОВНОГО ЕКРАНУ!", xpos, ypos);
    tft.unloadFont(); // выгрузка шрифта из памяти
    drawKeypad(keyLabel, keyColor);
    newDispl = false;
  }
} */
void displ_2(){
  if(newDispl){
    for (int i = 0; i < MENU_1; i++) {
      keyLabel[i] = labelsMenu1[i];
      keyColor[i] = colorsMenu1[i];
    }
    tft.fillScreen(TFT_BLACK);
    // tft.loadFont("Arial28"); // загрузка в память шрифта
    // tft.setTextDatum(TC_DATUM);
    tft.drawString("канал 2", 160, 5);
    drawKeypad_longName(keyLabel, keyColor, MENU_1, 1);
    // tft.unloadFont(); // выгрузка шрифта из памяти

    newDispl = false;
  }
}
/* void displ_2(void){
  // Create 15 keys for the keypad
  if(newDispl){
    for (int i = 0; i < 15; i++) {
      keyLabel[i] = labels_for_display_2[i];
      keyColor[i] = color_for_display_2[i];
    }
    tft.fillScreen(TFT_BLACK);
    xpos = 5; ypos = 0;
    tft.loadFont("Calibri14"); // загрузка в память шрифта
    uint16_t h = tft.fontHeight();
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(xpos, ypos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.drawString(txt2_for_display_3[0], xpos, ypos); tft.drawString(txt2_for_display_3[1], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt2_for_display_3[2], xpos, ypos); tft.drawString(txt2_for_display_3[3], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt2_for_display_3[4], xpos, ypos); tft.drawString(txt2_for_display_3[5], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt2_for_display_3[6], xpos, ypos); tft.drawString(txt2_for_display_3[7], xpos+170, ypos);
    ypos += h;
    tft.drawString(txt2_for_display_3[8], xpos, ypos); tft.drawString(txt2_for_display_3[9], xpos+170, ypos);
    ypos += h;
    tft.setTextColor(TFT_BLACK, TFT_YELLOW, true);
    tft.drawString("#90-ПОВЕРНЕННЯ ДО ПОПЕРЕДНЬОГО ЕКРАНУ!", xpos, ypos);
    ypos += h+1;
    tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
    tft.drawString("#99-ПОВЕРНЕННЯ ДО ГОЛОВНОГО ЕКРАНУ!", xpos, ypos);
    tft.unloadFont(); // выгрузка шрифта из памяти
    drawKeypad(keyLabel, keyColor);
    newDispl = false;
  }
} */

void displ_3(void){
  // Create 15 keys for the keypad
  if(newDispl){
    for (int i = 0; i < 15; i++) {
      keyLabel[i] = labels_for_display_3[i];
      keyColor[i] = color_for_display_3[i];
    }  
    // Draw keypad background
    tft.fillScreen(TFT_DARKGREY);
    // Draw number display area and frame
    tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
    drawKeypad(keyLabel, keyColor);
    newDispl = false;
  }
}


/* void display(void){
  switch (displNum){
  	case 0: displ_0(); break;//- СТАН КАМЕРИ --
  	case 1: displ_1(); break;//- НАЛАШТУВАННЯ  1-12 -
    case 2: displ_2(); break;//- НАЛАШТУВАННЯ 13-20 -
    // case 3: displ_3(); break;//- КАЛЬКУЛЯТОР -
    // case 4: displ_4(); break;//- ЗМІНА РЕЖИМУ -
    // case 5: displ_5(); break;//- ІНШЕ -
    // case 6: displ_6(); break;//- ЗМІНА ІНШЕ -
    // case 7: displ_7(); break;//- вибір ШВИДКІСТІ обертання -
    // case 8: displ_8(); break;//- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання -
  	// default: displ_0();	break;//- СТАН КАМЕРИ -
  }
} */

void drawKeypad_longName(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  const char* nul = "";
  for (uint8_t row = 0; row < amt_row; row++) {
    for (uint8_t col = 0; col < amt_col; col++) {
      uint8_t b = col + row * amt_col;
// TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize
      key[b].initButton(&tft, 160 + col * (300 + 5),
                        50 + row * (30 + 5), // x, y, w, h, outline, fill, text
                        300, 30, TFT_WHITE, keyColor[b], TFT_BLACK,
                        (char*)nul, KEY_TEXTSIZE);
      String string = String(keyLabel[b]);
      key[b].drawButton(false, string);
    }
  }
}
void drawKeypad(const char* keyLabel[], uint16_t keyColor[]){
  tft.setFreeFont(LABEL2_FONT);
  for (uint8_t row = 0; row < 3; row++) {
    for (uint8_t col = 0; col < 5; col++) {
      uint8_t b = col + row * 5;
// TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize
      key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_BLACK,
                        (char*)keyLabel[b], KEY_TEXTSIZE);
      key[b].drawButton();
    }
  }
}

uint16_t lampUpdate(uint16_t xpos, uint16_t ypos){
    uint16_t txt_width, h;
    bool on = false;
    tft.loadFont("Calibri14"); // загрузка в память шрифта
    h = tft.fontHeight()+4;
    tft.fillRect(xpos-10, ypos-4, 300, h+4, TFT_DARKGREY);
    tft.drawRect(xpos-10, ypos-4, 300, h+4, TFT_MAGENTA);
    tft.setCursor(xpos, ypos);
    on = seconds&1 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_YELLOW, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print("  НАГРІВ  ");
    txt_width = tft.textWidth("  НАГРІВ  ");
    xpos += txt_width+10;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&2 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_CYAN, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ЗВОЛОЖ ");
    txt_width = tft.textWidth(" ЗВОЛОЖ ");
    xpos += txt_width+10;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&4 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREEN, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ПОВОРОТ ");
    txt_width = tft.textWidth(" ПОВОРОТ ");
    xpos += txt_width+10;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&8 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_ORANGE, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ДОПОМІЖ ");
    txt_width = tft.textWidth(" ДОПОМІЖ ");
    xpos += txt_width+10;
    ypos += h;
    tft.unloadFont(); // выгрузка шрифта из памяти
    return ypos;
}

#include "main.h"
#include "display.h"
#include "tftArcFill.h"

//--------- ОСНОВНОЙ ЭКРАН ----------------------
void displ_0(void){
  uint16_t h,w;
//-----------
  if(grafDispl[0].value != ds[0].pvT) {
    grafDispl[0].value = ds[0].pvT;
    diagram(grafDispl[0], TFT_WHITE);
  }
  if(grafDispl[1].value != ds[1].pvT) {
    grafDispl[1].value = ds[1].pvT;
    diagram(grafDispl[1], TFT_WHITE);
  }
//-----------
  h = lampUpdate(20, 130);
//-----------
  xpos = 0; ypos = h+8;
  tft.loadFont("Arial20"); // загрузка в память шрифта
  tft.setTextDatum(TL_DATUM);
  h = tft.fontHeight();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(displStr,"РЕЖИМ: Р=%g  І=%g", pid[0].pPart, pid[0].iPart);
  w = tft.textWidth("РЕЖИМ:");
  tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);

  xpos = 0; ypos += (h+3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  sprintf(displStr,"ПОВОРОТ: %3d сек.",seconds);
  w = tft.textWidth("ПОВОРОТ:");
  tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);

  xpos = 0; ypos += (h+3);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprintf(displStr,"ІНКУБАЦІЯ: %3d сек.",seconds);
  w = tft.textWidth("ІНКУБАЦІЯ:");
  tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  tft.unloadFont(); // выгрузка шрифта из памяти
}

void displ_1(void){
  // Create 15 keys for the keypad
  const char* keyLabel[15] = {"#1","#2","#3","#4","#5","#6","#7","#8","#9","#10","#11","#12","#13","#14","#15"};
  uint16_t keyColor[15] = {TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN,
                          TFT_RED, TFT_RED, TFT_BLUE, TFT_BLUE, 
                          TFT_CYAN, TFT_CYAN, TFT_GREEN, TFT_GREEN,
                          TFT_BLUE, TFT_BLUE, TFT_WHITE
                          };
  if(newDispl){
    uint16_t h;
    tft.fillScreen(TFT_BLACK);
    xpos = 0; ypos = 0;
    tft.loadFont("Calibri14"); // загрузка в память шрифта
    h = tft.fontHeight();
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(xpos, ypos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.drawString("#1-завдання нагрівача;     #2-завдання зволожувача;", xpos, ypos);
    ypos += h+0;
    tft.drawString("#3-лотки вгору;                     #4-лотки униз;", xpos, ypos);
    ypos += h+1;
    tft.drawString("#5-авар. відхилення Т1;    #6-авар. відхилення Т2;", xpos, ypos);
    ypos += h+1;
    tft.drawString("#7-охолодж. увімкнено;    #8-охолодж. вимкнено;", xpos, ypos);
    ypos += h+1;
    tft.drawString("#9-провітр. увімкнено;       #10-провітр. вимкнено;", xpos, ypos);
    ypos += h+1;
    tft.drawString("#11-заслінка закрита;         #12-заслінка відкрита;", xpos, ypos);
    ypos += h+1;
    tft.drawString("#13-положення заслінки;   #14-програма інкубації;", xpos, ypos);
    ypos += h+1;
    tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
    tft.drawString("#15-ПОВЕРНЕННЯ ДО ГОЛОВНОГО ЕКРАНУ!", xpos, ypos);
    tft.unloadFont(); // выгрузка шрифта из памяти
    drawKeypad(keyLabel, keyColor);
    newDispl = false;
  }
}

void display(void){
  switch (diplNum){
  	case 0: displ_0(); break;//- СТАН КАМЕРИ --
  	case 1: displ_1(); break;//- НАЛАШТУВАННЯ -
    // case 2: displ_2(); break;//- НАЛАШТУВАННЯ -
    // case 3: displ_3(); break;//- ЗМІНА ТЕМПЕРАТУР -
    // case 4: displ_4(); break;//- ЗМІНА РЕЖИМУ -
    // case 5: displ_5(); break;//- ІНШЕ -
    // case 6: displ_6(); break;//- ЗМІНА ІНШЕ -
    // case 7: displ_7(); break;//- вибір ШВИДКІСТІ обертання -
    // case 8: displ_8(); break;//- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання -
  	default: displ_0();	break;//- СТАН КАМЕРИ -
  }
}

void drawKeypad(const char* keyLabel[], uint16_t keyColor[]){
  TFT_eSPI_Button key[16];
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
    uint16_t txt_width, txt_height;
    bool on = false;
    tft.loadFont("Calibri14"); // загрузка в память шрифта
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
    txt_height = tft.fontHeight()+5;
    ypos += txt_height;
    tft.unloadFont(); // выгрузка шрифта из памяти
    return ypos;
}

/* void strUpdate(const char* txt, uint16_t* xpos, uint16_t ypos, uint16_t* txt_h){
    uint16_t txt_w;
    tft.loadFont("Arial20"); // загрузка в память шрифта
    tft.setCursor(*xpos, ypos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(txt);
    txt_w = tft.textWidth(txt);
    *xpos += txt_w+10;
    *txt_h = tft.fontHeight()+5;
    tft.unloadFont(); // выгрузка шрифта из памяти
} */

/* void strPrint(const char* txt, uint16_t* xpos, uint16_t ypos, uint16_t* txt_h){
  uint16_t w, h = tft.fontHeight();
  tft.setCursor(*xpos, ypos);
  sprintf(displStr,"ІНКУБАЦІЯ %3d сек.",seconds);
  w = tft.textWidth("ІНКУБАЦІЯ");
  tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  // tft.print(displStr);
  tft.drawString(displStr, xpos, ypos);
  tft.unloadFont(); // выгрузка шрифта из памяти
} */
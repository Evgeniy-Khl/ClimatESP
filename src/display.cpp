#include "main.h"
#include "display.h"
#include "tftArcFill.h"

//--------- ОСНОВНОЙ ЭКРАН ----------------------
void displ_0(void)
{
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
  txt_height = lampUpdate(20, 130);
//-----------
  xpos = 0; ypos = txt_height+5;
  strUpdate("РЕЖИМ", &xpos, ypos, &txt_height);
  sprintf(displStr,"P=%g  I=%g          ", pid.pPart, pid.iPart);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(displStr, xpos, ypos-5, 4);

  xpos = 0; ypos += txt_height;
  strUpdate("ПОВОРОТ", &xpos, ypos, &txt_height);
  sprintf(displStr,"%3d sek.",seconds);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(displStr, xpos, ypos-5, 4);

  xpos = 0; ypos += txt_height;
  strUpdate("ІНКУБАЦІЯ", &xpos, ypos, &txt_height);

/* 
  Y_str = Y_top+15;  // 15
  const char* point[3] = {"  ","  ","  "};
  uint32_t curTime = sTime.Hours*3600 + sTime.Minutes*60 + sTime.Seconds;
  if(WORK){
//    if(INSIDE) point[1] = "->";
    if(set[TMR0]) point[2] = "->";
    else  point[0] = "->";
  }
  if(NEWBUTT){
    GUI_Clear(fillScreen);
    initializeButtons(3,1,40);// 3 колонки; одна строка; высота 40
    if(WORK|VENTIL|PURGING) drawButton(MAGENTA, 0, "СТОП");
    else drawButton(GREEN, 0, "ПУСК");
    drawButton(YELLOW, 1, "Керуван.");
    drawButton(CYAN, 2, "Налаштув.");
  }
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  X_left = 15;
  if(WORK) GUI_WriteString(X_left, Y_str, " ON  ", Font_16x26, BLACK, GREEN);
  else if(VENTIL) GUI_WriteString(X_left, Y_str, "VENT ", Font_16x26, BLACK, YELLOW);
  else if(PURGING) GUI_WriteString(X_left, Y_str, "PURG ", Font_16x26, BLACK, CYAN);
  else {
    GUI_WriteString(X_left, Y_str, " OFF ", Font_16x26, YELLOW, RED);
    color0 = WHITE; color1 = WHITE;
  }
  
  GUI_WriteString(120, Y_str, "РЕЖИМ:", Font_11x18, YELLOW, fillScreen);
  sprintf(buffTFT,"%8s", modeName[modeCell]);
  GUI_WriteString(190, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
  Y_str = Y_str+26+15; //56
  //----------------------
  X_left = 20;
  if(errors & 0x01) GUI_WriteString(X_left, Y_str, " ПОМИЛКА  ", Font_11x18, YELLOW, RED);
  else if(errors & ERR3) GUI_WriteString(X_left, Y_str, " ПЕРЕГРIВ ", Font_11x18, YELLOW, RED);
  else if(errors & ERR5) GUI_WriteString(X_left, Y_str, "ВIДХIЛЕННЯ", Font_11x18, YELLOW, RED);
  else GUI_WriteString(X_left, Y_str, "  КАМЕРА  ", Font_11x18, YELLOW, fillScreen);
  //----------------------
  X_left = 180;
  if(errors & 0x02) GUI_WriteString(X_left, Y_str, " ПОМИЛКА  ", Font_11x18, YELLOW, RED);
  else if(errors & ERR4) GUI_WriteString(X_left, Y_str, " ПЕРЕГРIВ ", Font_11x18, YELLOW, RED);
  else GUI_WriteString(X_left, Y_str, "  ПРОДУКТ ", Font_11x18, YELLOW, fillScreen);
  //----------------------
  if(grafDispl[0].value != ds.pvT[0] || NEWBUTT) {
      grafDispl[0].value = ds.pvT[0];
      diagram(grafDispl[0], color0);
  }
  if(grafDispl[1].value != ds.pvT[1] || NEWBUTT) {
      grafDispl[1].value = ds.pvT[1];
      diagram(grafDispl[1], color1);
  }
  NEWBUTT = OFF;
  Y_str = 240;
  //-------------------------------------------------------------------------------------------
  X_left = 30;
  GUI_WriteString(X_left, Y_str, "   ТРИВАЛIСТЬ РЕЖИМУ   ", Font_11x18, YELLOW, fillScreen);
  Y_str = Y_str+18+15; // 204
  if(WORK|PURGING){
    sprintf(buffTFT,"%2s %02u:%02u:%02u ", point[2], sTime.Hours, sTime.Minutes, sTime.Seconds);
    GUI_WriteString(15, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
  }
  uint16_t tmr = set[TMR0];
  if(PURGING) {tmr = set[TMR1]; sprintf(buffTFT," %iхвл.%02iсек.", tmr/60, tmr%60);}
  else sprintf(buffTFT," %iгод.%02iхвл.", tmr/60, tmr%60);
  GUI_WriteString(165, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
  Y_str = Y_str+18+15;  // 237
  
  if(modeCell<3 && VENTIL && curTime>2 && curTime<12){
    ticBeep = 10;
    GUI_FillRectangle(42, Y_str, lcddev.width - 75, 60, RED);// Y_str = 344+56 = 400
    if(modeCell) GUI_WriteString(70, Y_str+5, "ЗАКРИЙТЕ ЗАСЛЫНКИ", Font_11x18, YELLOW, RED);
    else GUI_WriteString(65, Y_str+5, "ВЫДКРИЙТЕ ЗАСЛЫНКИ", Font_11x18, YELLOW, RED);
    GUI_WriteString(110, Y_str+35, "вентиляцыъ!", Font_11x18, YELLOW, RED);
//    Y_str = Y_str+18+15; // 270
  }
  else if(modeCell<3 && VENTIL && curTime>2 && curTime==12) GUI_FillRectangle(42, Y_str, lcddev.width - 75, 60, fillScreen); 
  else if(modeCell>1)
  {
    if(modeCell==2){
      sensor = T3; 
      if(errors & 0x0008) GUI_WriteString(80, Y_str, "ПОМИЛКА ДАТЧИКА", Font_11x18, YELLOW, RED);
      else GUI_WriteString(80, Y_str, "ВОЛОГИЙ ДАТЧИК ", Font_11x18, YELLOW, fillScreen);
    }
    else if(modeCell==3){
      sensor = T2;
      if(errors & 0x0004) GUI_WriteString(30, Y_str, "    ПОМИЛКА ДАТЧИКА    ", Font_11x18, YELLOW, RED);
      else if(errors & ERR6){
        if(set[sensor]*10 > ds.pvT[sensor]) GUI_WriteString(30, Y_str, "ДИМ НИЗЬКОЪ ТЕМПЕРАТУРИ", Font_11x18, YELLOW, RED);
        else  GUI_WriteString(30, Y_str, "ДИМ ВИСОКОЪ ТЕМПЕРАТУРИ", Font_11x18, YELLOW, RED);
      }
      else GUI_WriteString(30, Y_str, "       ДАТЧИК ДИМУ     ", Font_11x18, YELLOW, fillScreen);
    }
    Y_str = Y_str+18+15; // 270
    
    if(ds.pvT[sensor]<1000) sprintf(buffTFT,"%3.1f$ ",(float)ds.pvT[sensor]/10);
    else if(ds.pvT[sensor]<1270) sprintf(buffTFT,"%5d$ ", ds.pvT[sensor]/10);
    else sprintf(buffTFT," ---  ");
    GUI_WriteString(55, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
    sprintf(buffTFT,"%3i.0$ ", set[sensor]);
    GUI_WriteString(175, Y_str, buffTFT, Font_16x26, BLACK, WHITE);
    Y_str = Y_str+26+15;  // 311
  }
  
  if(VENTIL && curTime > 12){
    if(errors & ERR8) GUI_WriteString(30, Y_str, "  НЕ ПРАЦЮЭ ВЕНТИЛЯТОР  ", Font_11x18, YELLOW, RED);
    else {
      sprintf(buffTFT,"%12s: %4i об/хвл.", setName[4], speedData[set[VENT]][0]);
      GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
    }
  }  
  GUI_FillRectangle(0, 0, 1, 1, fillScreen);//??????????????????????????????????? */
}

void display(uint8_t){
  switch (displ_num){
  	case 0: displ_0(); break;//- СТАН КАМЕРИ --
  	// case 1: displ_1(); break;//- СТАН ВЫХОДІВ -
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

void strUpdate(const char* txt, uint16_t* xpos, uint16_t ypos, uint16_t* txt_h){
    uint16_t txt_w;
    tft.loadFont("Arial20"); // загрузка в память шрифта
    tft.setCursor(*xpos, ypos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(txt);
    txt_w = tft.textWidth(txt);
    *xpos += txt_w+10;
    *txt_h = tft.fontHeight()+5;
    tft.unloadFont(); // выгрузка шрифта из памяти
}
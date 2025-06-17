#include "main.h"
#include "TFT_eSPI.h"
#include "display.h"
#include "tftArcFill.h"
#include "touchKeypad.h"

const char* txt1_for_display_3[15] = {
"1-завдання нагрівача", "2-завдання зволожув.", "3-лотки вгору", "4-лотки униз",
"5-авар. відхилення Т1", "6-авар. відхилення Т2", "7-охолодж. увімкнено",
"8-охолодж. вимкнено", "9-провітр. увімкнено", "10-провітр. вимкнено", "11-положення заслінки",
"12-програма інкубації", "", "", ""
};
const char* txt2_for_display_3[15] = {
"13-заслінка закрита", "14-заслінка відкрита", "15-мінім. імпульс","16-максим. імпульс",
"17-період імпульсів", "18-аварійний режим", "19-режим роботи реле", "20-пропорц. коефіціент",
"21-ітеграл. коефіціент", "22-дифер. коефіціент", "", "", "", "", ""
};

char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex, numberDispl;
bool newTxt = false;

void checkKeypad(){
    // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 15; b++) {
    if (key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    // if (b < 3) tft.setFreeFont(LABEL1_FONT);
    // else tft.setFreeFont(LABEL2_FONT);

    if (key[b].justReleased()) key[b].drawButton();     // draw normal

    if (key[b].justPressed()) {
      key[b].drawButton(true);  // draw invert

        // Возврат к главному экрану
        if (b == 14) {
            switch (displNum){
                case 1: displNum = 0; break;
                case 2: displNum = 0; break;
                case 3: displNum = 0; break;    // Ok
                // default: displNum = 0; break;
            }
            newDispl = true;
            sprintf(displStr,"В==14: Номер=%d",displNum);
            status(displStr);
            delay(2000); // UI debouncing
        }

        if (b == 13) {
            switch (displNum){
                case 1: displNum = 2; break;
                case 2: displNum = 1; break;
                case 3: displNum = 1; break;
                // default: displNum = 0; break;
            }
            newDispl = true;
            sprintf(displStr,"В==13: Номер=%d",displNum);
            status(displStr);
            delay(2000); // UI debouncing
        }
        
        if(b < 13){
            newDispl = true;
            newTxt = true;
            switch (displNum){
                case 1: 
                        numberIndex = b;
                        numberDispl = displNum;
                        displNum = 3;
                break;
                case 2: 
                        numberIndex = b;
                        numberDispl = displNum;
                        displNum = 3;
                break;
                case 3: 
                        numberIndex = b;
                        numberDispl = displNum;
                        displNum = 3;
                        newDispl = false;
                break;
                // default: displNum = 0; break;
            }
            status("В<13");
            delay(2000); // UI debouncing
        }
    }
  }
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

// Print something in the mini status bar
void status(const char *msg) {
  tft.loadFont("Arial28"); // загрузка в память шрифта
  tft.setTextPadding(320);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
//   tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
//   tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
  tft.unloadFont(); // выгрузка шрифта из памяти
}
#include "main.h"
#include "TFT_eSPI.h"
#include "display.h"
#include "tftArcFill.h"
#include "touchKeypad.h"

const char* txt1_for_display_3[15] = {
"1-завдання нагрівача", "2-завдання зволожув.", "3-авар. відхилення Т1", "4-авар. відхилення Т2", "5-охолодж. увімкнено",
"6-охолодж. вимкнено", "7-лотки вгору", "8-лотки униз", "9-провітр. увімкнено", "10-провітр. вимкнено", "11-положення заслінки",
"12-програма інкубації", "", "", ""
};
const char* txt2_for_display_3[15] = {
"13-заслінка закрита", "14-заслінка відкрита", "15-мінім. імпульс","16-максим. імпульс",
"17-період імпульсів", "18-аварійний режим", "19-режим роботи реле", "20-пропорц. коефіціент",
"21-ітеграл. коефіціент", "22-дифер. коефіціент", "", "", "", "", ""
};

char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex, numberDispl;

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
    if (key[b].justReleased()) key[b].drawButton();     // draw normal
    if (key[b].isPressed()) {
      key[b].drawButton(true);  // draw invert
      switch (displNum){
        case 1:
          newDispl = true;
          if (b == 14) displNum = 0;
          else if (b == 13) displNum = 2;
          else {
            numberDispl = displNum;
            numberIndex = b;
            editValue = settings.flat_array[numberIndex]; 
            displNum = 3; 
            displ_3();
            window(0);
          }
          Serial.print("case 1: displNum="); Serial.println(displNum);
        break;
        case 2: 
          newDispl = true;
          if (b == 14) displNum = 0;
          else if (b == 13) displNum = 1;
          else {
            numberDispl = displNum;
            numberIndex = b;
            editValue = settings.flat_array[numberIndex];
            displNum = 3;
            displ_3();
            window(0);
          }
          Serial.print("case 2: displNum="); Serial.println(displNum);
        break;
        case 3: 
            int8_t v = butCheck(b);
            window(v);
          Serial.print("case 3: displNum="); Serial.println(displNum);
          switch (displNum){
            case 1: newDispl = true; displ_1(); break;//- НАЛАШТУВАННЯ  1-12 -
            case 2: newDispl = true; displ_2(); break;//- НАЛАШТУВАННЯ 13-20 -
          }
        break;
      }
      // status("В<13");
      // delay(2000); // UI debouncing
    }
  }
}

int8_t butCheck(uint8_t butt){
  const char* current_label = labels_for_display_3[butt];
  long value = 0;
  // 1. Проверяем на пустую строку
    if (strlen(current_label) == 0) {
      Serial.println("Пустая строка.");
    }

    // 2. Проверяем на известные команды
    if (strcmp(current_label, "X") == 0) {
      // Serial.println("Найдена команда 'Отмена' (X).");
      displNum = numberDispl;
    }
    if (strcmp(current_label, "Ok") == 0) {
      // Serial.println("Найдена команда 'Подтвердить' (Ok).");
      // Serial.print("numberIndex: "); Serial.println(numberIndex);
      // Serial.print("editValue: "); Serial.println(editValue);
      // Serial.print("settings.flat_array[numberIndex]: "); Serial.println(settings.flat_array[numberIndex]);
      settings.flat_array[numberIndex] = editValue;
      if(numberIndex == 0 || numberIndex == 16){
        grafDispl[0].sp = settings.sp_structs[0].spT;
        grafDispl[1].sp = settings.sp_structs[1].spT;
      }
      // Serial.print("new: "); Serial.println(settings.flat_array[numberIndex]);
      displNum = numberDispl;
    }
    // 3. Если это не команда и не пустая строка, пытаемся преобразовать в число
    char* end; // Указатель на символ, где остановился парсинг
    value = strtol(current_label, &end, 10); // 10 - десятичная система

    // strtol - умная функция. Если она смогла прочитать хоть что-то,
    // то указатель 'end' сдвинется с начала строки.
    // Если 'end' остался в начале, значит, строка не начинается с числа.
    // if (end != current_label) {
    //   Serial.print("Преобразовано в число: ");
    //   Serial.println(value);
    // } else {
    //   // Этот блок сработает, если в массиве появится неизвестная нечисловая строка
    //   Serial.println("Неизвестная строка (не число и не команда).");
    // }
    return value;
}

void window(int8_t val){
  uint8_t dividerValue = 1;
  if(numberIndex < 5) dividerValue = 10;
  editValue += val;
  newTxt = true;
  tft.loadFont("Arial28"); // загрузка в память шрифта
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(txt1_for_display_3[numberIndex], DISP_W/2, DISP_Y + 5);
  sprintf(displStr,"%5.1f  Д=%d  К=%i",editValue/dividerValue, dividerValue, val);
  tft.drawString(displStr, DISP_W/2, DISP_Y + 5 + 28);
  tft.unloadFont(); // выгрузка шрифта из памяти
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
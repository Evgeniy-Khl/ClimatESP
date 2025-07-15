#include "main.h"

void ledDisplKeypad(long now){
    //-------------------------------------------- 10 mSec. --------------------------------------
    if(now - counter10 > 10){
      counter10 = now;
      if(beepOn) beepOn--; else digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
      if(settings.sp_structs[0].mode == 4 && --pvPulse == 0){ // импульсный режим увлажнения
        humidiValue = TRIACOFF;
        writePCF8574(portOut.value);
      }
      keys = module.getButtons();
      if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}
    }

  //-------------------------------------------- КЛАВИАТУРА --------------------------------------
    if(now - counterWait > waitCheckKeyPad){
      counterWait = now;
      keys = module.getButtons();
      
      if(lastKey == keys && keys > 0){
        keyCount++;
        checkkey(keys);
        if(numSetup == 0) ledDispl(displNum);
        else display_setup();
        module.setDisplay(data, 8);
      } 
      else if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}
      else lastKey = keys;
    }
  //============================= НОВАЯ ПОЛ-СЕКУНДА =================================

    if(now - counter1s > 500){
        counter1s = now; 
        if(++halfSecond > 119) halfSecond = 0; 
        // if(resetDispl) --resetDispl;
        // else if(numSetup) saveset();  // сохраняем установки
        // else displNum = 0;            // возврат к главному дисплею
        if(numSetup == 0) ledDispl(displNum);
        else display_setup();
        module.setDisplay(data, 8);
    }
}

void ledSet(void){
    byte led = ~portOut.value;
    if(pctHeater == 100) led |= 1;
    else if(pctHeater > 10){led &= 0xFE; led |= halfSecond&1;}
    else led &= 0xFE;
    if(pctHimidifier == 100) led |= 2;
    else if(pctHimidifier > 10){led &= 0xFD; led |= (halfSecond&1)<<1;}
    else led &= 0xFD;
    // if(errorsFlag.value) led
    for (uint8_t i = 0; i < 6; i++){
        module.setLED(led&1, i);
        led >>= 1;
    }
}
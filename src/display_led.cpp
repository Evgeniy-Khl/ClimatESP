#include "main.h"
#include "display_led.h"

uint8_t data[8];

void displ_top(signed int val, unsigned char comma){
  uint8_t i = 0, neg = 0, endComa = 0;
  if((ERROR4 || comma == 3) && (halfSecond & 1)){for (i=0; i<3; i++) data[i]=BL;} // мигают цифры
  else {
    if(comma == 1) comma = 0x80; else if(comma == 3){comma = 0; endComa = 0x80;}
    if(val<0) {neg = 1; val = -val;}
    if(val<1000){
      if (neg){
        if (val<100){
          data[0] = DEF;
          data[1] = NUMBER_FONT[(val/10)%10]|comma; // запятая
          data[2] = NUMBER_FONT[val%10];
        } else {
          data[0] = GR;  // -> -
          data[1] = GR;  // -> -
          data[2] = GR;  // -> -
        };
      } else {
        if(val/100) data[0] = NUMBER_FONT[val/100];
        else data[0] = 0;
        if(comma || val > 9) data[1] = NUMBER_FONT[(val/10)%10]|comma; // запятая
        else if(val < 10) data[1] = 0;
        data[2] = NUMBER_FONT[val%10]|endComa;
      };
    } else {
      data[0] = DEF;  // -> -
      data[1] = DEF;  // -> -
      data[2] = DEF;  // -> -
    };
  }
}

void displ_bot(signed int val, unsigned char comma){
  uint8_t i = 0, neg = 0, endComa = 0;
  if((ERROR8 || comma > 1)&&(halfSecond & 1)){for (i=3; i<6; i++) data[i]=BL;} // мигают цифры
  else {
    if(comma == 1) comma=0x80;
    else if(comma == 3){comma = 0; endComa = 0x80;}
    else {i = comma; comma = 0;}
    if(val<0) {neg = 1; val = -val;}
    if(val<1000){
      if (neg){
        if (val<100) {
          data[3] = DEF;
          data[4] = NUMBER_FONT[(val/10)%10]|comma; // запятая
          data[5] = NUMBER_FONT[val%10];
        } else {
          data[0] = GR;  // -> o
          data[1] = GR;  // -> o
          data[2] = GR;  // -> o
        };
      } else {
        if(i == 2){
          if(val < 16) data[3] = YY;  // "У"
          else {data[3] = PE; val -= 15;}  // "П"
        } 
        else if(val/100) data[3] = NUMBER_FONT[val/100];
        else data[3] = 0;
        data[4] = NUMBER_FONT[(val/10)%10]|comma; // запятая
        data[5] = NUMBER_FONT[val%10]|endComa;
      };
    } else {
      data[3] = DEF;  // -> -
      data[4] = DEF;  // -> -
      data[5] = DEF;  // -> -
    };
  }
}

void clr_top(void){
  unsigned char byte;
  for (byte=0; byte<3; byte++) data[byte]=BL;
}

void clr_bot(void){
  unsigned char byte;
  for (byte=3; byte<6; byte++) data[byte]=BL;
}

void displ_67(signed int val, unsigned char mode){
  uint8_t neg=0;
  if(val<0) {neg=1; val=-val;}
  if(neg){
    if(val<10){
        data[6] = DEF;
        data[7] = NUMBER_FONT[val%10];
    } else {
        data[6] = GR; // o
        data[7] = GR; // o
    };
  } else if(val<99){
      data[6] = NUMBER_FONT[(val/10)&0x0F];
      data[7] = NUMBER_FONT[(val%10)&0x0F];
      if(val<10){
        switch (mode){
          case ERRORS:  data[6] = EE; break;
          case DAY:     data[6] = DD; break;
          case COOL:    data[6] = GR; break;
          default:      data[6] = BL;
        }
      }
  }
  else {data[6] = DEF; data[7] = DEF;}; // --
}

//===================== ledDisplay ========================
void ledDispl(unsigned char mode){
  switch (mode){
        //-------------t0;----------------------------RH;            /              t1; --------------------Timer;-------------------
    case 0:
      displ_top(ds[0].pvT,COMMA); 
      if(HIH5030) displ_bot(pvRH,COMMA); else displ_bot(ds[1].pvT,COMMA);
      if(errorsFlag.value) displ_67(errorsFlag.value, ERRORS); 
      else if(COOLING || AERATION) displ_67(pvVenting, COOL);
          // else if(programm) displ_678(date,DAY); 
      else if(HIH5030) displ_67(pvTimer, NOCOMMA); 
      else displ_67(halfSecond / 2, NOCOMMA); //pvRH
      break;
       //-------------------t1;----------------------tNTC;--------------------"F2"---------
    case 1: 
      if(HIH5030) displ_top(ds[1].pvT, COMMA); 
      else displ_top(pvTimer, NOCOMMA); 
      clr_bot();      // displ_bot(pvTTriac, NOCOMMA); 
      data[6]=FF; data[7]=NUMBER_FONT[2]; 
      break;
       //-------------------Flap;--------------------date;--------------------"F3"---------
    case 2: 
      displ_top(settings.sp_structs[0].state, NOCOMMA); 
      displ_bot(0,NOCOMMA); 
      data[6]=FF; data[7]=NUMBER_FONT[3]; 
      break;
       //---------------уставка t0;-------------------------уставка RH;-----------------------уставка t1-----------------"F4"---------
    case 3: 
      displ_top(settings.sp_structs[0].spT, COMMA); 
      if(HIH5030) displ_bot(settings.sp_structs[1].spRH, COMMA); 
      else displ_bot(settings.sp_structs[1].spT, COMMA);
      data[6]=FF; data[7]=NUMBER_FONT[4];
      break;
 }
 if (OVERHEAT){
    data[6] = PE; data[7] = GE;
    if(halfSecond & 1) {for (uint8_t i=0; i<8; i++) data[i] = BL;}; // мигание дисплея при перегреве симистора
  };
}

//==================== Setup ========================
void display_setup(void){
  // errorsFlag = 0;
  if(editBuff>999) editBuff=999; else if(editBuff<-99) editBuff=-99;
  if(numSetup==3||numSetup==4||numSetup==5||numSetup==6||(numSetup>=15 && numSetup<21)||numSetup>21){
    displ_top(editBuff,NOCOMMA); displ_bot(numSetup,2);               //Верхний дисплей + Запятая
  } 
  else {
    displ_top(editBuff,COMMA); displ_bot(numSetup,2);              //Верхний дисплей 
  }
  if(numSetup > 15) {data[6] = PE; data[7] = NUMBER_FONT[10];}  // "ПА"
  else {data[6] = YY; data[7] = NUMBER_FONT[12];}  // "УС"
}

void displ_IP(void){
    //-------------------------- ошибки MOUNTING FS ------------------------------
    if(dataLed[4] || dataLed[5]){         
      data[6] = NUMBER_FONT[dataLed[4]];
      data[7] = NUMBER_FONT[dataLed[5]];
      module.setDisplay(data, 8);
      digitalWrite(BEEP_PIN, LOW); // Включаем бипер
      delay(1000);
      digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
      delay(2000);
    }
    //------------------------------ индикация IP (первая пара) --------------------------------
    displ_top(dataLed[0],ENDCOMMA);
    displ_bot(dataLed[1],ENDCOMMA);
    displ_67(1, NOCOMMA);
    module.setDisplay(data, 8);
    digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    delay(3000);
    //------------------------------ индикация IP (вторая пара) --------------------------------
    displ_top(dataLed[2],ENDCOMMA);
    displ_bot(dataLed[3],ENDCOMMA);
    displ_67(2, NOCOMMA);
    module.setDisplay(data, 8);
    digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    delay(3000);
    //-------------------------------- индикация марки прибора ---------------------------------
    data[0] = NUMBER_FONT[numberOfDevices]; // отображение числа датчиков на дисплее
    data[1] =  0b01011110; // d
    data[2] =  0b00000110; // 1
    data[3] =  0b11101101; // 5.
    data[4] =  0b01011011; // 2
    data[5] =  0b01101101; // 5
    data[6] =  0;          // blank
    data[7] =  0;          // blank
    module.setDisplay(data, 8);
    digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    delay(100);
    digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
}
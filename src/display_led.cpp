#include "main.h"
#include "display_led.h"

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
        data[6] = DEF; data[7] = NUMBER_FONT[val%10];
    } else {
        data[6] = DEF; data[7] = DEF;
    };
  } else if(val<10){
      switch (mode){
          case ERRORS:  data[6] = EE; break;
          case DAY:     data[6] = DD; break;
          case COOL:    data[6] = GR; break;
          default:      data[6] = BL;
        }
  } else if(val<100){
      data[6] = NUMBER_FONT[(val/10)&0x0F];
      data[7] = NUMBER_FONT[(val%10)&0x0F];
  } else if(val==100){
      data[6] = GR; data[7] = GR; // oo
  } else {
      data[6] = HH; data[7] = HH; // HH
  }
}

//===================== ledDisplay ========================
void ledDispl(unsigned char mode){
  switch (mode){
        //------------- pvT0;    	pvT1;    			pvRH / ERRORS / AERATION --------------------
    case 0:
      displ_top(ds[0].pvT,COMMA);
      if(HIH5030 || SENSOR_DHT22) {displ_bot(pvRH, NOCOMMA); displ_67(-100, NOCOMMA);}  // "--"
      else {displ_bot(ds[1].pvT,COMMA); displ_67(pvRH, NOCOMMA);}
      if(errorsFlag.value) displ_67(errorsFlag.value, ERRORS); 
      else if(AERATION) displ_67(pvVenting, COOL);
      else displ_67(pvRH, NOCOMMA); // pctHeater -> power %
      break;
       //-------------- pvT1;      pvT2;    			"F1"---------
    case 1: 
      displ_top(ds[1].pvT,COMMA); 
      displ_bot(ds[2].pvT,COMMA); 
      displ_bot(settings.sp_structs[0].state, NOCOMMA); 
      data[6]=FF; data[7]=NUMBER_FONT[mode]; 
      break;
       //-------------- pvTimer;   pvFlap;    		"F2"---------
    case 2: 
      displ_top(pvTimer, NOCOMMA);
      displ_bot(settings.sp_structs[0].state, NOCOMMA); 
      data[6]=FF; data[7]=NUMBER_FONT[mode]; 
      break;
       //-------------- state;     date;    			"F3"---------
    case 3: 
      displ_top(settings.sp_structs[1].state, NOCOMMA);
      displ_bot(0, NOCOMMA); 
      data[6]=FF; data[7]=NUMBER_FONT[mode]; 
      break;
       //-------------- spT0;    	spT1 / spRH;		"F4"---------
    case 4: 
      displ_top(settings.sp_structs[0].spT, COMMA); 
      if(HIH5030) displ_bot(settings.sp_structs[1].spRH, COMMA); 
      else displ_bot(settings.sp_structs[1].spT, COMMA);
      data[6]=FF; data[7]=NUMBER_FONT[mode];
      break;
       //-------------- IP0;		IP1;				"F5" -------------
    case 5:
      if(WIFIENABLE){
        IPAddress myIP = WiFi.localIP();
        displ_top(myIP[0],ENDCOMMA);
        displ_bot(myIP[1],ENDCOMMA);
        displ_67(1, NOCOMMA);
      } else {
        displ_top(0,ENDCOMMA);
        displ_bot(0,ENDCOMMA);
        displ_67(1, NOCOMMA);
      }
      break;
       //-------------- IP2;		IP3;				"F6" -------------
    case 6:
      if(WIFIENABLE){
        IPAddress myIP = WiFi.localIP();
        displ_top(myIP[2],ENDCOMMA);
        displ_bot(myIP[3],ENDCOMMA);
        displ_67(2, NOCOMMA);
      } else {
        displ_top(0,ENDCOMMA);
        displ_bot(0,ENDCOMMA);
        displ_67(2, NOCOMMA);
      }
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

//--------------------- ИНДИКАЦИЯ ОШИБОК и IP адреса ----------------------------
void displErrors(void){
    int8_t duration = 0;
    for (uint8_t i = 0; i < 8; i++) { data[i] = DEF;}
    if(dataLed[0]) {data[2] = NUMBER_FONT[dataLed[0]]; duration++;}     //"--1 --- --" ошибка RTC не найдена!
    if(dataLed[1]) {data[3] = NUMBER_FONT[dataLed[1]]; duration +=2;}   //"--- 1-- --" ошибка RTC lost power!
    if(dataLed[2]) {data[4] = NUMBER_FONT[dataLed[2]]; duration +=4;}   //"--- -1- --" ошибка checkSetpoint
    if(dataLed[3]) {data[5] = NUMBER_FONT[dataLed[3]]; duration +=6;}   //"--- --1 --" ошибка checkConfig
    if(dataLed[4]) {data[6] = NUMBER_FONT[15]; duration +=8;}           //"--- --- F-" ошибка MOUNTING FS
    if(dataLed[5]) {data[7] = NUMBER_FONT[12]; duration =127;}          //"--- --- -C" ошибка writePCF8574
    module.setDisplay(data, 8);
    do {
      digitalWrite(BEEP_PIN, LOW); // Включаем бипер
      delay(500);
      digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
      delay(2000);
      duration--;
    } while (duration > 0);
    //------------------------------ индикация IP (первая пара) --------------------------------
    // if(WIFIENABLE){
    //   IPAddress myIP = WiFi.localIP();
      
    //   displ_top(myIP[0],ENDCOMMA);
    //   displ_bot(myIP[1],ENDCOMMA);
    //   displ_67(1, NOCOMMA);
    //   module.setDisplay(data, 8);
    //   digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    //   delay(100);
    //   digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    //   delay(3000);
    //   //------------------------------ индикация IP (вторая пара) --------------------------------
    //   displ_top(myIP[2],ENDCOMMA);
    //   displ_bot(myIP[3],ENDCOMMA);
    //   displ_67(2, NOCOMMA);
    //   module.setDisplay(data, 8);
    //   digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    //   delay(100);
    //   digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    //   delay(100);
    //   digitalWrite(BEEP_PIN, LOW); // Включаем бипер
    //   delay(100);
    //   digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
    //   delay(3000);
    // }
    //-------------------------------- индикация марки прибора ---------------------------------
    for (uint8_t i = 0; i < 8; i++) { data[i] = OO;}                    //"ooo ooo oo"

    
    switch (detectedSensor){
    case SENSOR_DS18B20: data[0] = NUMBER_FONT[numberOfDevices]; break; //4oo ooo oo
    case SENSOR_DHT22:   data[1] = NUMBER_FONT[1]; break;               //o1o ooo oo
    case UNKNOWN: 
          data[0] = NUMBER_FONT[0];                                     //00o ooo oo
          data[1] = NUMBER_FONT[0];
          // TO DO ОСТВНОВИТЬ ПРОГРАММУ!
      break;
    }
    if(HIH5030) data[2] = NUMBER_FONT[1];                               //xx1 ooo oo
    if(RTCENABLE) data[3] = NUMBER_FONT[1];                             //xxx 1oo oo
    if(WIFIENABLE) data[4] = NUMBER_FONT[1];                            //xxx x1o oo

    data[6] = NUMBER_FONT[0];                                           // версия v.00
    data[7] = NUMBER_FONT[0];

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
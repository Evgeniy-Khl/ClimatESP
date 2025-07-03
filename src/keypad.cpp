#include "keypad.h"

uint8_t checkkey(uint8_t keydata){
  uint8_t byte, crc, topUser=30, topOwner=30, botUser=0;
  static unsigned char key, count;
  if(key == keydata) ++count;
  else if(key == 0){count = 6; key = keydata;}
  else {count = 0; key = keydata;};
  if (count>5){
    count = 0;
    if(key){       // если нажата любая кнопка
      beepOn = 25; digitalWrite(BEEP_PIN, LOW); // Включаем бипер
      if(numSetup){   // режим РЕДАКТИРОВАНИЯ УСТАВОК И ПАРАМЕТРОВ
        resetDispl = 10; // удерживаем режим установок 10 сек.
        switch (key){
           case KEY_1:
             {
              waitkey = WAITCOUNT;
              if (++numSetup > topOwner) numSetup=1;         // Меню пользователя
              switch (numSetup)
                {
                 case 1: editBuff = settings.sp_structs[0].spT; break;          // У1 уставка канал 1
                 case 2: if(HIH5030) 
                              editBuff = settings.sp_structs[1].spRH;           // У2 уставка канал 2 
                         else editBuff = settings.sp_structs[1].spT; 
                    break;
                 case 3: editBuff = settings.sp_structs[0].timer; break;        // У3 время отключенного состояния
                 case 4: editBuff = settings.sp_structs[1].timer; break;        // У4 время включенного состояния (если не 0 то это секунды)
                 case 5: editBuff = settings.sp_structs[0].aeration; break;     // У5 ПАУЗА ПРОВЕТРИВАНИЯ (минут)
                 case 6: editBuff = settings.sp_structs[1].aeration; break;     // У6 ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
                 case 7: editBuff = settings.sp_structs[0].coolOn; break;       // У7 включение охлаждения по каналу 1
                 case 8: editBuff = settings.sp_structs[0].coolOff; break;      // У8 включение охлаждения по каналу 1
                 case 9: editBuff = settings.sp_structs[1].coolOn; break;       // У9 включение охлаждения по каналу 2
                 case 10: editBuff = settings.sp_structs[1].coolOff; break;     // У10 включение охлаждения по каналу 2
                 case 11: editBuff = settings.sp_structs[0].alarm; break;       // У11 тревога по каналу 1
                 case 12: editBuff = settings.sp_structs[1].alarm; break;       // У12 тревога по каналу 2
                 case 13: editBuff = settings.sp_structs[0].auxiliary; break;   // У13 включение вспомогательного канала
                 case 14: editBuff = settings.sp_structs[1].auxiliary; break;   // У14 выключение вспомогательного канала
                 case 15: editBuff = settings.sp_structs[0].state; break;       // У15 текущее положение заслонки
                //  case 16: editBuff = settings.sp_structs[1].state; break;       // У16 номер программы
                //  case 17: editBuff = settings.sp_structs[0].spRH; break;        // У17 подстройка датчика HIH-5030-01
                 /* case 14:                               // У14 подстройка датчика DS18B20
                        if(devices==1)
                         { 
                          try = 0;
                          do {
                              w1_init(); w1_write(0xCC);// 1 Wire Bus initialization; Skip ROM [CCH] command
                              w1_write(0xBE); // Read Scratchpad command [BE]
                              ptr_to_ram = ds_buffer;
                              for (byte=0; byte < 8; byte++){*ptr_to_ram++ = w1_readnew();}
                              crc = w1_readnew(); // Read CRC byte
                              ptr_to_ram = ds_buffer;
                              if(w1_dow_crc8(ptr_to_ram, 8)==crc){try = 2; if(ds_buffer[2]==TUNING) editBuff=(signed char)ds_buffer[3]; else editBuff=0;}
                             } while (++try < 2);
                         } else editBuff=999; break;
                 case 15: editBuff=offSetRH;   break;        // У15 смещение выключения увлажнения */
                }; 
             } break;
           case KEY_2:{editBuff++; if (waitkey) waitkey--;} break;
           case KEY_3:
             {
              waitkey=WAITCOUNT;
              ++numSetup;
              if (numSetup > topUser || numSetup < botUser) numSetup = botUser;// Меню специалиста
              switch (numSetup)
                {
                 case 16: editBuff = 0; break;                 // П0 сброс параметров
                 case 17: editBuff = settings.sp_structs[1].mode; break;          // П1 = 0 задержка регулировки по влажному
                 case 18: editBuff = settings.sp_structs[0].extendMode; break;        // П2 = 0 режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; 2-УПРАВЛЕНИЕ ВСПОМОГАТЕЛЬНЫМ НАГРЕВАТЕЛЕМ
                 case 19: editBuff = settings.sp_structs[0].mode; break;         // П3 = MINRELAYMODE релейный режим работы
                 case 20: editBuff = settings.sp_structs[0].pulse; break;        // П4 = 200 - 1,0 сек.
                //  case 21: editBuff=maxRun/DEN; break;        // П5 = 2000- 10,0 сек.
                 case 22: editBuff = settings.sp_structs[1].pulse; break;        // П6 = 3000-  15 сек.
                //  case 23: editBuff=offSetHeater; break;      // П7 = 5
                 case 26: editBuff = settings.sp_structs[0].flapLimit; break;          // П10 = 40 close
                 case 27: editBuff = settings.sp_structs[1].flapLimit; break;          // П11 = 85 open
                 case 28: editBuff = settings.sp_structs[0].Kp; break;          // П12 = 20
                 case 29: editBuff = settings.sp_structs[0].Ki; break;         // П13 = 500
                 case 30: editBuff = settings.sp_structs[1].Kp; break;          // П14 = 15
                 case 31: editBuff = settings.sp_structs[1].Ki; break;         // П15 = 900                 
                };
             } break;
           case KEY_4:{editBuff--; if (waitkey) waitkey--;} break;
           default: waitkey=WAITCOUNT;
          }; 
       }
      /* else if (setprgday)       // режим СОСТАВЛЕНИЕ ПРОГРАММЫ
       {
        waitset=10;             // удерживаем режим установок
        drafting_prog(key);     // составление программы
       } */
      else                      // режим ИЗМЕРЕНИЯ
       {
        switch (key)
          {
           case KEY_1:     numSetup = 1; editBuff = settings.sp_structs[0].spT; resetDispl = 5; break;
           case KEY_2:     if(settings.sp_structs[1].timer) {pvTimer=settings.sp_structs[1].timer;} 
                           else {pvTimer=settings.sp_structs[0].timer;} 
                           TURN = ON;
                break;
          //  case KEY_3:     if(errors && disableBeep==0) {disableBeep=10; key=255;} else {Check = 1; ++displmode; displmode&=3; waitset=20;}; break;
           case KEY_4:     pvTimer=settings.sp_structs[0].timer; TURN = OFF; break;
           case KEY_5:     EXTRA1 = ON; break;
           case KEY_6:     EXTRA2 = ON; break;
          //  case KEY_4_3_2: pwTriac1=maxRun; CN2 = CN2ON; break;
          //  case KEY_5_2:   pvVenting+=10; DoAeration=1; beepOn=150; waitkey=WAITCOUNT*3; break;               // ПРОВЕТРИВАНИЕ начато 
          //  case KEY_5_4:   pvWait=aeration[0]; DoAeration=0; pvFlap=flpNow; break;                               // ПРОВЕТРИВАНИЕ закончено
          //  case KEY_8_6_5: date = start(); if (programm) prg_stepoint(date,1); break;                        // старт новой инкубации
          //  case KEY_7:     if(programm){setprgday=1; read_prg(setprgday, programm); waitset=15;} break;      // просмотр, редактирование пр-мы.
          };
       };
      if (numSetup){
        // if(numSetup&0x10)  displ_buffer[6] = PE; else  displ_buffer[6] = YY;
        data[7] = NUMBER_FONT[(numSetup&0x0F)/10]; data[8] = NUMBER_FONT[(numSetup&0x0F)%10];
       };
     }
    else waitkey=WAITCOUNT;
   };
  return key;
}

void saveset(void){
 char byte;
 switch (numSetup)//---------------------- Меню пользователя ---------------------------------------
   {
    case 1: settings.sp_structs[0].spT = editBuff; break;
    case 2: if(HIH5030) settings.sp_structs[1].spRH = editBuff; 
            else settings.sp_structs[1].spT = editBuff; 
        break;
    case 3: settings.sp_structs[0].timer = editBuff; pvTimer = settings.sp_structs[0].timer; break;// длительность выключенного состояния таймера
    case 4: settings.sp_structs[1].timer = editBuff; break;
    case 5: settings.sp_structs[0].aeration = editBuff; break;                                     // ПАУЗА ПРОВЕТРИВАНИЯ (минут)
    case 6: settings.sp_structs[1].aeration = editBuff;
            if(settings.sp_structs[1].aeration){
              pvVenting = settings.sp_structs[1].aeration; 
              pvAeration = 0; AERATION=1;} 
        break;  // ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
    // case 7: editBuff &= 0xFF; if(editBuff<MIN_OFFSET) editBuff=MIN_OFFSET; settings.sp_structs[0].offSet=editBuff; break;  // ограничено 2 - 255-> 25,5 грд.С. или 25,5% RH смещение для CN4
    // case 8: editBuff &= 0xFF; if(editBuff<MIN_OFFSET) editBuff=MIN_OFFSET; settings.sp_structs[1].offSet=editBuff; break;  // ограничено 2 - 255-> 25,5 грд.С. или 25,5% RH смещение для CN4
    case 9:  editBuff &= 0xFF; if(editBuff < 2) editBuff = 2; settings.sp_structs[0].alarm = editBuff; break;               // ограничено 2 - 255;
    case 10: editBuff &= 0xFF; if(editBuff < 2) editBuff = 2; settings.sp_structs[1].alarm = editBuff; break;               // ограничено 2 - 255;
    case 11: editBuff &= 0x7F; if(editBuff < 0) editBuff = 0; 
             settings.sp_structs[0].flapLimit = settings.sp_structs[0].state = editBuff; //setflap(); 
        break;       // ограничено 0 - 127 текущее положение заслонки
    // case 12:
    //   {
    //    if(editBuff>4) editBuff=4; else if(editBuff<0) editBuff=0;// № программы->ограничено 0 - 4
    //    programm = editBuff;
    //    rw_twi(READ,DS1307N,0,clock_buffer);
    //    if (programm) clock_buffer[7]=SQWE_1Hz; // индикация режима "ПРОГРАММА ВКЮЧЕНА"
    //    else clock_buffer[7]=OUT_1;             // индикация OFF
    //    rw_twi(WRITE,DS1307N,0,clock_buffer);
    //    if (programm) prg_stepoint(date,1);
    //   } break;                    
    case 13: if(editBuff>100) editBuff=100; 
             else if(editBuff<-100) editBuff=-100; 
             settings.sp_structs[0].spRH=editBuff; 
        break;      // подстройка датчика HIH
    // case 14:                                  // подстройка датчика DS18B20
    //  if(devices==1)// только если подключен 1 датчик 
    //   {
    //    if (editBuff>10) editBuff=10; else if (editBuff<-10) editBuff=-10; // 10-> 1грд.С.
    //    byte = editBuff;
    //    w1_init(); w1_write(0xCC); w1_write(0x4E);// Skip ROM [CCH] command; Write Scratchpad command [4E]
    //    w1_write(0xAA); w1_write(byte); w1_write(0x7F);// TH; TL; Configuration Register.
    //    w1_init(); w1_write(0xCC); w1_write(0x48); // Copy Scratchpad command [48]
    //   } break;
    // case 15: editBuff&=0x7F; offSetRH = editBuff; break; // смещение выключения увлажнения (ограничено 0 - 127-> 12,7 грд.С. или 12,7% RH)
    //------------------------------------ П00 -------------------------------------------------------------------
    // case 16: switch (editBuff) {
    //           case  1: topOwner=MAXOWNER1; break;// разрешение вводить У11 текущее положение заслонки и У12 номер программы
    //           case  5: topOwner=MAXOWNER5; break;// разрешение вводить У13 подстройка HIH-5030-01 и У14 подстройка DS18B20
    //           case 10: topUser=MAXOWNER10; break;// разрешение вводить П7 
    //           case 15: topUser=TOPKOFF; botUser=BOTKOFF; break;// разрешение вводить корекцию коэфициентов cof[3];
    //           case 31: reset();                break;// сброс параметров
    //                       }; 
    // break;//--------------------------- Меню специалиста ---------------------------------------------------------
    case 17: if(editBuff) settings.sp_structs[1].mode = 1; 
             else settings.sp_structs[1].mode = 0; 
        break;               // задержка регулировки по влажному
    case 18: settings.sp_structs[0].extendMode = editBuff /*& MAXEXTMODE*/; break; // режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; 2-УПРАВЛЕНИЕ ВСПОМОГАТЕЛЬНЫМ НАГРЕВАТЕЛЕМ
    // case 19: if(editBuff>MAXRELAYMODE) editBuff=MAXRELAYMODE; else if(editBuff<MINRELAYMODE) editBuff=MINRELAYMODE; relayMode=editBuff;
    //          if(relayMode==4) topUser=PULSMENU; else topUser=TOPUSER; break;//релейный режим работы
    case 20: if(editBuff < 1) editBuff = 1; 
             editBuff &= 0x3F;
             settings.sp_structs[0].pulse = editBuff/**DEN*/; 
        break;       // ограничено 0.1-6.3 секунд;
    // case 21: editBuff &= 0xFF;  if(editBuff<1) editBuff=1; maxRun=editBuff*DEN; break;       // ограничено 0.1-25.5 секунд;
    case 22: if(editBuff < 5) editBuff = 5; 
             editBuff &= 0x3FF;
             settings.sp_structs[1].pulse = editBuff/**200*/; 
        break;       // ограничено 5-999 секунд (16 мин.39 сек.);
    // case 23: editBuff &= 0x1F;  
    //          if(editBuff < 2) editBuff = 2; offSetHeater=editBuff; 
    //     break;     // ограничено 0,2-3,2 грд. С.;
    case 26: editBuff &= 0x3F; settings.sp_structs[0].flapLimit = editBuff; /*setflap();*/ break;                // close->ограничено 0 - 63
    case 27: editBuff &= 0x7F; settings.sp_structs[1].flapLimit = editBuff;  /*setflap();*/ break;                // open ->ограничено 0 - 127
    case 28: editBuff &= 0x03F; if(editBuff<1) editBuff=1; settings.sp_structs[0].Kp = editBuff; break;         // ограничено 1 - 63;
    case 29: editBuff &= 0x3FF; if(editBuff<100) editBuff=100; settings.sp_structs[0].Ki = editBuff; break;    // ограничено 100 - 1023;
    case 30: editBuff &= 0x0FF; if(editBuff<1) editBuff=1; settings.sp_structs[1].Kp = editBuff; break;         // ограничено 1 - 255;
    case 31: editBuff &= 0x3FF; if(editBuff<100) editBuff=100; settings.sp_structs[1].Ki = editBuff; break;    // ограничено 100 - 1023;
   }; 
 numSetup=0;
}

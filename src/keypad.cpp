#include "keypad.h"

void checkkey(uint8_t key){
  uint8_t topUser=31, topOwner=15, botUser=16;
  beepOn = 25; digitalWrite(BEEP_PIN, LOW); // Включаем бипер
  if(numSetup){   //==== режим РЕДАКТИРОВАНИЯ УСТАВОК И ПАРАМЕТРОВ ======
    resetDispl = RESETDISPLAY; // удерживаем режим установок 10 сек.
    switch (key){
        case KEY_1:
                    waitCheckKeyPad = WAITCHECKKEYPAD ;
                    if (++numSetup > topOwner) numSetup=1;         // Меню пользователя
                    switch (numSetup){
                        case 1: editBuff = settings.sp_structs[0].spT; break;          // У1 уставка канал 1
                        case 2: if(HIH5030) editBuff = settings.sp_structs[1].spRH;    // У2 уставка канал 2 
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
                        case 15: editBuff = settings.sp_structs[1].state; break;       // У15 номер программы
                      }; 
          break;
        case KEY_2:{editBuff++; if(waitCheckKeyPad > 100) waitCheckKeyPad -= 25;} break;
        case KEY_3:
                    waitCheckKeyPad = WAITCHECKKEYPAD;
                    ++numSetup;
                    if (numSetup > topUser || numSetup < botUser) numSetup = botUser;// Меню специалиста
                    switch (numSetup){
                        case 16: editBuff = settings.sp_structs[0].state; break;          // У15 текущее положение заслонки
                        case 17: editBuff = settings.sp_structs[1].mode; break;           // П1 = 0 задержка регулировки по влажному
                        case 18: editBuff = settings.sp_structs[0].mode; break;           // П3 = MINRELAYMODE релейный режим работы
                        case 19: editBuff = settings.sp_structs[0].extendMode; break;     // П2 = 0 режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; 2-УПРАВЛЕНИЕ ВСПОМОГАТЕЛЬНЫМ НАГРЕВАТЕЛЕМ
                        case 20: editBuff = settings.sp_structs[1].extendMode; break;     // П2 = 0 режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; 2-УПРАВЛЕНИЕ ВСПОМОГАТЕЛЬНЫМ НАГРЕВАТЕЛЕМ
                        case 21: editBuff = settings.sp_structs[0].pulse; break;          // П4 = 200 - 1,0 сек.
                        case 22: editBuff = settings.sp_structs[1].pulse; break;          // П6 = 3000-  15 сек.
                        case 23: editBuff = settings.sp_structs[0].flapLimit; break;      // П10 = 40 close
                        case 24: editBuff = settings.sp_structs[1].flapLimit; break;      // П11 = 85 open
                        case 25: editBuff = settings.sp_structs[0].Kp; break;             // П12 = 20
                        case 26: editBuff = settings.sp_structs[0].Ki; break;             // П13 = 500
                        case 27: editBuff = settings.sp_structs[1].Kp; break;             // П14 = 15
                        case 28: editBuff = settings.sp_structs[1].Ki; break;             // П15 = 900 
                        case 29: editBuff = settings.sp_structs[0].spRH; break;           // У17 подстройка датчика HIH-5030-01
                        case 30: editBuff = 0; break;                 // П0 сброс параметров
                    };
          break;
        case KEY_4:{editBuff--; if(waitCheckKeyPad > 100) waitCheckKeyPad -= 25;} break;
        default:    waitCheckKeyPad = WAITCHECKKEYPAD;
      }; 
  }
      /* else if (setprgday)       // режим СОСТАВЛЕНИЕ ПРОГРАММЫ
       {
        waitset=10;             // удерживаем режим установок
        drafting_prog(key);     // составление программы
       } */
  else {  //==================== ОСНОВНОЙ РЕЖИМ РАБОТЫ =================================
    switch (key) {
        case KEY_1: numSetup = 1; editBuff = settings.sp_structs[0].spT; resetDispl = RESETDISPLAY; break;
        case KEY_2: if(settings.sp_structs[1].timer) {pvTimer=settings.sp_structs[1].timer;} 
                    else {pvTimer=settings.sp_structs[0].timer;} 
                    TURN = ON;
            break;
        case KEY_3: if(++displNum > 4) displNum = 0;
                    resetDispl = RESETDISPLAY;
            break;
        case KEY_4: pvTimer=settings.sp_structs[0].timer; TURN = OFF; break;
        case KEY_5:                         break;
        case KEY_6:                         break;
        case KEY_7: if(numSetup) saveset(); break;
        #ifdef DEBUG
          case KEY_7_1: errorsFlag.value = 0; ERROR1 = 1; break;
          case KEY_7_2: errorsFlag.value = 0; ERROR2 = 1; break;
          case KEY_7_3: errorsFlag.value = 0; ERROR4 = 1; break;
          case KEY_7_4: errorsFlag.value = 0; ERROR8 = 1; break;
          case KEY_7_5: errorsFlag.value = 0; ERROR10 = 1; break;
          case KEY_7_6: errorsFlag.value = 0; FROZE = 1;  break;
          case KEY_7_8: errorsFlag.value = 0; OVERHEAT = 1; break;
          case KEY_8: errorsFlag.value = 0; break;
        #else
        case KEY_8: if(errors && disableBeep==0) disableBeep=RESETDISPLAY;
        #endif
          //  case KEY_4_3_2: pwTriac1=maxRun; CN2 = CN2ON; break;
          //  case KEY_5_2:   pvVenting+=10; DoAeration=1; beepOn=150; waitCheckKeyPad = WAITCHECKKEYPAD; break;               // ПРОВЕТРИВАНИЕ начато 
          //  case KEY_5_4:   pvWait=aeration[0]; DoAeration=0; pvFlap=flpNow; break;                               // ПРОВЕТРИВАНИЕ закончено
          //  case KEY_8_6_5: date = start(); if (programm) prg_stepoint(date,1); break;                        // старт новой инкубации
          //  case KEY_7:     if(programm){setprgday=1; read_prg(setprgday, programm); waitset=15;} break;      // просмотр, редактирование пр-мы.
    };
  };
}

void saveset(void){
  switch (numSetup){ //---------------------- Меню пользователя ---------------------------------------
      case 1: settings.sp_structs[0].spT = editBuff; break;       // НЕ ограничено
      case 2: if(HIH5030) 
                 settings.sp_structs[1].spRH = editBuff;          // ограничено +9.9 до 99,9
              else settings.sp_structs[1].spT = editBuff;         // ограничено +9.9 до 99,9
        break;
      case 3: settings.sp_structs[0].timer = editBuff;            // ограничено 0 до 999 мин.
              pvTimer = settings.sp_structs[0].timer; 
        break;  // длительность выключенного состояния таймера
      case 4: settings.sp_structs[1].timer = editBuff; break;     // ограничено 0 до 999 сек.
      case 5: settings.sp_structs[0].aeration = editBuff; break;  // ограничено 0 до 999 мин. ПАУЗА ПРОВЕТРИВАНИЯ
      case 6: settings.sp_structs[1].aeration = editBuff;         // ограничено 0 до 999 сек.
              if(editBuff){pvVenting = editBuff; pvAeration = 0; AERATION=1;} 
        break;  // ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
      case 7: settings.sp_structs[0].coolOn =  editBuff; break;   // ограничено 2 до 50 (0,2 до 5,0 Ц.)
      case 8: settings.sp_structs[0].coolOff = editBuff; break;   // ограничено 0 до 50 (0,0 до 5,0 Ц.)
      case 9:  settings.sp_structs[1].coolOn =  editBuff; break;  // ограничено 2 до 100 (0,2 до 10,0 Ц. 0,2% до 10%)
      case 10: settings.sp_structs[1].coolOff = editBuff; break;  // ограничено 0 до 100 (0,0 до 10,0 Ц. 0,0% до 10%)
      case 11: settings.sp_structs[0].alarm = editBuff;  break;   // ограничено 5 до 200 (0,5 до 20,0 Ц.)
      case 12: settings.sp_structs[1].alarm = editBuff;  break;   // ограничено 5 до 300 (0,5 до 30,0 Ц. 0,5% до 30%)
      case 13: settings.sp_structs[0].auxiliary = editBuff; break;  // ограничено 5 до 200 (0,5 до 20,0 Ц.)
      case 14: settings.sp_structs[1].auxiliary = editBuff; break;  // ограничено 0 до 200 (0,0 до 20,0 Ц.)
      case 15: settings.sp_structs[1].state = editBuff; break;    // НЕ ограничено программа текущая
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
    // case 13: if(editBuff>100) editBuff=100; 
    //          else if(editBuff<-100) editBuff=-100; 
    //          settings.sp_structs[0].spRH=editBuff; 
    //     break;      // подстройка датчика HIH
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
      case 16: settings.sp_structs[0].state = editBuff; break;  // ограничено 0 - 100% текущее положение заслонки
      case 17: if(editBuff) settings.sp_structs[1].mode = 1;    // НЕ ограничено задержка регулировки по влажному
               else settings.sp_structs[1].mode = 0; 
        break;
      case 18: settings.sp_structs[0].mode = editBuff;          // ограничено 0 - 4/ релейный 0-НЕТ; 1->по кан.0 2->по кан.1 3->по кан.0&1; 4-импульсное
              //  if(editBuff == 4) topUser=PULSMENU; else topUser=TOPUSER; 
        break;
      case 19: if(editBuff) settings.sp_structs[0].extendMode = 1;
               else settings.sp_structs[0].extendMode = 0;
        break; // режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
      case 20: settings.sp_structs[1].extendMode = editBuff; break; // ?????????????????????????????????
      case 21: settings.sp_structs[0].pulse = editBuff; break;      // ограничено 0.1-6.3 секунд;
      case 22: settings.sp_structs[1].pulse = editBuff; break;      // ограничено 5-999 секунд (16 мин.39 сек.);
      case 23: settings.sp_structs[0].flapLimit = editBuff; /*setflap();*/ break; // close->ограничено 0 - 63
      case 24: settings.sp_structs[1].flapLimit = editBuff;  /*setflap();*/ break;// open ->ограничено 0 - 127
      case 25: if(editBuff<1) editBuff=1; settings.sp_structs[0].Kp = editBuff;
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki);
        break;  // ограничено 1 - 999;
      case 26: settings.sp_structs[0].Ki = editBuff; 
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki)
        break;  // ограничено 0 - 999;
      case 27: if(editBuff<1) editBuff=1; settings.sp_structs[1].Kp = editBuff; 
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki)
        break;  // ограничено 1 - 999;
      case 28: settings.sp_structs[1].Ki = editBuff; 
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki)
        break;  // ограничено 0 - 999;
      case 29: settings.sp_structs[0].spRH = editBuff; break;   // ограничено -99 до 99 (-9,9 до 9,9 Ц.)
  };
  saveConfig();
  numSetup=0;
}

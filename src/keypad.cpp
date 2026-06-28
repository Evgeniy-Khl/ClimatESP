#include "keypad.h"

void checkkey(uint8_t key){
  // uint8_t topUser=31, topOwner=15, botUser=16;
  beeperOn(5);     // Включаем бипер
  if(numSetup){     //==== режим РЕДАКТИРОВАНИЯ УСТАВОК И ПАРАМЕТРОВ ======
    resetDispl = RESETDISPLAY; // удерживаем режим установок 10 сек.
    switch (key){
        case KEY_1: waitCheckKeyPad = WAITCHECKKEYPAD;
                    if (++numSetup > 15) numSetup=1;         // Меню пользователя
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
        case KEY_2: if(keyCount == 1) waitCheckKeyPad = WAITCHECKKEYPAD;
                    editBuff++; 
                    if(waitCheckKeyPad > MINWAIT) waitCheckKeyPad -= 100; 
                    switch (numSetup){
                      case 1:  if(editBuff > 999) editBuff = 999;  break;
                      case 2:  if(editBuff > 999) editBuff = 999;  break;
                      case 3:  if(editBuff > 999) editBuff = 999;  break;
                      case 4:  if(editBuff > 999) editBuff = 999;  break;
                      case 5:  if(editBuff > 999) editBuff = 999;  break;
                      case 6:  if(editBuff > 999) editBuff = 999;  break;
                      case 7:  if(editBuff > 200) editBuff = 200;  break;
                      case 8:  if(editBuff > 150) editBuff = 150;  break;
                      case 9:  if(editBuff > 200) editBuff = 200;  break;
                      case 10: if(editBuff > 150) editBuff = 150;  break;
                      case 11: if(editBuff > 400) editBuff = 400;  break;
                      case 12: if(editBuff > 400) editBuff = 400;  break;
                      case 13: if(editBuff > 400) editBuff = 400;  break;
                      case 14: if(editBuff > 400) editBuff = 400;  break;
                      case 15: if(editBuff >   4) editBuff =   4;  break;
                      case 16: if(editBuff > 100) editBuff = 100;  break;
                      case 17: if(editBuff >   4) editBuff =   4;  break;
                      case 18: if(editBuff >   3) editBuff =   3;  break;
                      case 19: if(editBuff >   1) editBuff =   1;  break;
                      case 20: if(editBuff >   3) editBuff =   3;  break;
                      case 21: if(editBuff > 100) editBuff = 100;  break;
                      case 22: if(editBuff >  60) editBuff =  60;  break;
                      case 23: if(editBuff >  50) editBuff =  50;  break;
                      case 24: if(editBuff > 100) editBuff = 100;  break;
                      case 25: if(editBuff > 999) editBuff = 999;  break;
                      case 26: if(editBuff > 999) editBuff = 999;  break;
                      case 27: if(editBuff > 999) editBuff = 999;  break;
                      case 28: if(editBuff > 999) editBuff = 999;  break;
                      case 29: if(editBuff > 999) editBuff = 999;  break;
                      case 30: if(editBuff >   3) editBuff =   3;  break;
                      case 31: if(editBuff >  15) editBuff =  15;  break;
                    }
          break;
        case KEY_3: waitCheckKeyPad = WAITCHECKKEYPAD;
                    ++numSetup;
                    if (numSetup > 31 || numSetup < 16) numSetup = 16;// Меню специалиста
                    switch (numSetup){
                        case 16: editBuff = settings.sp_structs[0].state; break;          // П01 текущее положение заслонки
                        case 17: editBuff = settings.sp_structs[0].mode; break;           // П02 = MINRELAYMODE релейный режим работы
                        case 18: editBuff = settings.sp_structs[1].mode; break;           // П03 = 0 задержка регулировки по влажному; 2-УВЛАЖНИТЕЛЬ
                        case 19: editBuff = settings.sp_structs[0].extendMode; break;     // П04 = 0 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
                        case 20: editBuff = settings.sp_structs[1].extendMode; break;     // П05 = 1 1-ОХЛАЖДЕНИЕ; 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ
                        case 21: editBuff = settings.sp_structs[0].pulse / 10; break;     // П06 = 0.1 - 10,0 сек.
                        case 22: editBuff = settings.sp_structs[1].pulse; break;          // П07 = 3000-  15 сек.
                        case 23: editBuff = settings.sp_structs[0].flapLimit; break;      // П08 = 40 close
                        case 24: editBuff = settings.sp_structs[1].flapLimit; break;      // П09 = 85 open
                        case 25: editBuff = settings.sp_structs[0].Kp; break;             // П10 = 100
                        case 26: editBuff = settings.sp_structs[0].Ki; break;             // П11 = 400
                        case 27: editBuff = settings.sp_structs[1].Kp; break;             // П12 = 100
                        case 28: editBuff = settings.sp_structs[1].Ki; break;             // П13 = 400 
                        case 29: editBuff = settings.sp_structs[0].spRH; break;           // П14 подстройка датчика HIH-5030-01
                        case 30: editBuff = settings.sp_structs[0].special & 0x03; break; // П15 таймаут WiFi маска 0х03
                        case 31: editBuff = settings.sp_structs[1].special & 0x0F; break; // П16 номер прибора маска 0х0F
                    };
          break;
        case KEY_4: if(keyCount == 1) waitCheckKeyPad = WAITCHECKKEYPAD;
                    editBuff--; 
                    if(waitCheckKeyPad > MINWAIT) waitCheckKeyPad -= 100; 
                    switch (numSetup){
                      case 1:   if(editBuff <  10) editBuff =  10;  break;
                      case 2:   if(editBuff <  10) editBuff =  10;  break;
                      case 3:   if(editBuff <   1) editBuff =   1;  break;
                      case 4:   if(editBuff <   0) editBuff =   0;  break;
                      case 5:   if(editBuff <   1) editBuff =   1;  break;
                      case 6:   if(editBuff <   0) editBuff =   0;  break;
                      case 7:   if(editBuff <   1) editBuff =   1;  break;
                      case 8:   if(editBuff <   0) editBuff =   0;  break;
                      case 9:   if(editBuff <   1) editBuff =   1;  break;
                      case 10:  if(editBuff <   0) editBuff =   0;  break;
                      case 11:  if(editBuff <   5) editBuff =   5;  break;
                      case 12:  if(editBuff <   5) editBuff =   5;  break;
                      case 13:  if(editBuff <   5) editBuff =   5;  break;
                      case 14:  if(editBuff <   5) editBuff =   5;  break;
                      case 15:  if(editBuff <   0) editBuff =   0;  break;
                      case 16:  if(editBuff <   0) editBuff =   0;  break;
                      case 17:  if(editBuff <   0) editBuff =   0;  break;
                      case 18:  if(editBuff <   0) editBuff =   0;  break;
                      case 19:  if(editBuff <   0) editBuff =   0;  break;
                      case 20:  if(editBuff <   1) editBuff =   1;  break;
                      case 21:  if(editBuff <   1) editBuff =   1;  break;
                      case 22:  if(editBuff <   5) editBuff =   5;  break;
                      case 23:  if(editBuff <   0) editBuff =   0;  break;
                      case 24:  if(editBuff <  50) editBuff =  50;  break;
                      case 25:  if(editBuff <   1) editBuff =   1;  break;
                      case 26:  if(editBuff <   0) editBuff =   0;  break;
                      case 27:  if(editBuff <   1) editBuff =   1;  break;
                      case 28:  if(editBuff <   0) editBuff =   0;  break;
                      case 29:  if(editBuff < -99) editBuff = -99;  break;
                      case 30:  if(editBuff <   0) editBuff =   0;  break;
                      case 31:  if(editBuff <   1) editBuff =   1;  break;
                    }
          break;
        //==== режим РЕДАКТИРОВАНИЯ УСТАВОК И ПАРАМЕТРОВ ======
        case KEY_5_4_6: reset(); break;             // Сброс установок к значениям по умолчанию
        // Очищает область памяти в AT24C32, используемую для хранения суточных данных. Заполняет нулями 288 записей (t1 и t2).
        case KEY_5_4_2: clearEEPROM(false); break;  // с анимацией и звуком
        // Запись программы инкубации по умолчанию
        case KEY_7_1: prepareProg1(); beeperOn(100); break;
        case KEY_7_2: prepareProg2(); beeperOn(100); break;
        case KEY_7_3: prepareProg3(); beeperOn(100); break;
        case KEY_7_4: prepareProg4(); beeperOn(100); break;
        // wifiManager.resetSettings()
        case KEY_7_4_6: beeperOn(50); settings.sp_structs[0].special |= 0x08; saveSetpoint(); ESP.restart(); break;
        // Удаляем только файлы графиков и статистики (безопасная очистка)
        case KEY_7_4_6_8: beeperOn(50); module.setDisplay(CLEAN_DATA, 8); clearIncubationData(); delay(1000); break;
        case KEY_8: saveset(); break;
        default:    waitCheckKeyPad = WAITCHECKKEYPAD;
      }; 
  }
  else {  //==================== ОСНОВНОЙ РЕЖИМ РАБОТЫ =================================
    waitCheckKeyPad = WAITCHECKKEYPAD;
    switch (key) {
        // Вход в режим Установок
        case KEY_1: numSetup = 1; editBuff = settings.sp_structs[0].spT; resetDispl = RESETDISPLAY; break;
        // TURN = ON
        case KEY_2: if(settings.sp_structs[1].timer) {pvTimer=settings.sp_structs[1].timer;} 
                    else {pvTimer=settings.sp_structs[0].timer;} 
                    TURN = PCF_ON;
                    writePCF8574(portOut.value);
            break;
        // Переключение дисплеев "F1","F2","F3","F4","F5"->IP0;IP1;	"F6"->IP2;IP3;
        case KEY_3: if(++displNum > 6) displNum = 0;
                    resetDispl = RESETDISPLAY;
            break;
        // TURN = OFF
        case KEY_4: pvTimer=settings.sp_structs[0].timer; 
                    TURN = PCF_OFF;
                    writePCF8574(portOut.value);
            break;
        case KEY_5: break;
        case KEY_6: break;
        case KEY_7: break;
        case KEY_8: if(errors && disableBeep==0) disableBeep=RESETDISPLAY;
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
      case 1: settings.sp_structs[0].spT = editBuff; break;       // У1 уставка канал 1 (0.1 - 99.9 °C)
      case 2: if(HIH5030) 
                 settings.sp_structs[1].spRH = editBuff;          // У2 уставка канал 2 (0.1 - 99.9 %)
              else settings.sp_structs[1].spT = editBuff;         // У2 уставка канал 2 (0.1 - 99.9 °C)
        break;
      case 3: settings.sp_structs[0].timer = editBuff;            // У3 время отключенного состояния (1 - 999 мин.)
              pvTimer = editBuff; 
        break;  // длительность выключенного состояния таймера
      case 4: settings.sp_structs[1].timer = editBuff; break;     // У4 время включенного состояния (0 - 999 сек.)
      case 5: settings.sp_structs[0].aeration = editBuff; break;  // У5 ПАУЗА ПРОВЕТРИВАНИЯ (1 - 999 мин.)
      case 6: settings.sp_structs[1].aeration = editBuff;         // У6 ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (0 - 999 сек.)
              if(editBuff){pvVenting = editBuff; pvAeration = 0; AERATION=1;} 
        break;
      case 7: settings.sp_structs[0].coolOn =  editBuff; break;   // У7 включение охлаждения по каналу 1 (0,1 - 20,0 °C)
      case 8: settings.sp_structs[0].coolOff = editBuff; break;   // У8 включение охлаждения по каналу 1 (0,0 - 15,0 °C)
      case 9:  settings.sp_structs[1].coolOn =  editBuff; break;  // У9 включение охлаждения по каналу 2 (0,1 - 20,0 °C/%)
      case 10: settings.sp_structs[1].coolOff = editBuff; break;  // У10 включение охлаждения по каналу 2 (0,0 - 15,0 °C/%)
      case 11: settings.sp_structs[0].alarm = editBuff;  break;   // У11 тревога по каналу 1 (0,5 - 40,0 °C)
      case 12: settings.sp_structs[1].alarm = editBuff;  break;   // У12 тревога по каналу 2 (0,5 - 40,0 °C/%)
      case 13: settings.sp_structs[0].auxiliary = editBuff; break;  // У13 включение вспомогательного канала (0,5 - 40,0 °C)
      case 14: settings.sp_structs[1].auxiliary = editBuff; break;  // У14 выключение вспомогательного канала (0,5 - 40,0 °C)
      case 15: 
          if ((settings.sp_structs[1].state != editBuff) && WIFIENABLE) {// если нет WiFi установка программы невозможна
            settings.sp_structs[1].state = editBuff;
            startIncubation();
          }
          break;    // У15 номер программы (0 - 4)
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
    
    // case 14:                                  // подстройка датчика DS18B20
    //  if(devices==1)// только если подключен 1 датчик 
    //   {
    //    if (editBuff>10) editBuff=10; else if (editBuff<-10) editBuff=-10; // 10-> 1грд.С.
    //    byte = editBuff;
    //    w1_init(); w1_write(0xCC); w1_write(0x4E);// Skip ROM [CCH] command; Write Scratchpad command [4E]
    //    w1_write(0xAA); w1_write(byte); w1_write(0x7F);// TH; TL; Configuration Register.
    //    w1_init(); w1_write(0xCC); w1_write(0x48); // Copy Scratchpad command [48]
    //   } break;
    
    //--------------------------- Меню специалиста ---------------------------------------------------------
      case 16: settings.sp_structs[0].state = editBuff; break;  // П01 ограничено 0 - 100% текущее положение заслонки
      case 17: settings.sp_structs[0].mode = editBuff;  // П03 ограничено 0 - 4 (релейный 0-НЕТ; 1->по кан.0 2->по кан.1 3->по кан.0&1; 4-импульсное)
              //  if(editBuff == 4) topUser=PULSMENU; else topUser=TOPUSER; 
        break;
      case 18: settings.sp_structs[1].mode = editBuff;  break;  // П02 0 - 3 задержка регулировки по влажному; 2-УВЛАЖНИТЕЛЬ
      case 19: settings.sp_structs[0].extendMode = editBuff; break; // П04 режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
      case 20: settings.sp_structs[1].extendMode = editBuff; break; // П05 1-ОХЛАЖДЕНИЕ; 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ
      case 21: settings.sp_structs[0].pulse = editBuff * 10; break; // П06 ограничено 1 - 100 (0.1 - 10.0) секунд;
      case 22: settings.sp_structs[1].pulse = editBuff; break;      // П07 ограничено 5-60 секунд
      case 23: settings.sp_structs[0].flapLimit = editBuff; /*setflap();*/ break; // П08 close->ограничено 0 - 63
      case 24: settings.sp_structs[1].flapLimit = editBuff;  /*setflap();*/ break;// П09 open ->ограничено 0 - 127
      case 25: settings.sp_structs[0].Kp = editBuff;  // П10
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki); // Kp/4; Ki/10000;
        break;  // ограничено 1 - 999;
      case 26: settings.sp_structs[0].Ki = editBuff;  // П11
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki); // Kp/4; Ki/10000;
        break;  // ограничено 0 - 999;
      case 27: settings.sp_structs[1].Kp = editBuff;  // П12
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki); // Kp/4; Ki/10000;
        break;  // ограничено 1 - 999;
      case 28: settings.sp_structs[1].Ki = editBuff;  // П13
              //  PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki); // Kp/4; Ki/10000;
        break;  // ограничено 0 - 999;
      case 29: settings.sp_structs[0].spRH = editBuff; break;   // П14 ограничено -99 до 99 (-9,9 до 9,9 Ц.)
      case 30: settings.sp_structs[0].special = editBuff & 0x03; break;// П15 ограничено 0 до 3
      case 31: settings.sp_structs[1].special = editBuff & 0x0F; break;// П16 ограничено 0 до 15
  };
  saveSetpoint();
  numSetup=0;
}

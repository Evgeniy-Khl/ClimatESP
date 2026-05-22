#include "programm.h"

TableBuff unBuf;

/**
 * AT24C32 имеет страницы по 32 байта. 
 * адреса страниц: 0-31, 32-63, ... 
 * 0xF00 - 0xFFF резерв 255 байт.
 */
uint16_t eepromMemoryAddressForDay(uint8_t prg, uint8_t day){
	// из расчета одна запись на 1 день (16 байт), 1 прог.=31 день
    uint16_t addressPage = PROGRAMS_START + (day - 1) * 16 + (prg - 1) * (16 * 31);  
	return addressPage;
}

void applyDailyProgram() {
    uint8_t prg = settings.sp_structs[1].state;
    if (prg == 0) return; // Если программа не запущена, ничего не делаем

    uint8_t day = countDays + 1; // Внутренний счетчик начинается с 0, в EEPROM дни с 1 до 31
    if (day > 31) {
        MYDEBUG_PRINTLN("applyDailyProgram: Инкубация завершена или день вне диапазона (>31).");
        return;
    }

    uint16_t address = eepromMemoryAddressForDay(prg, day);
    uint16_t bytesRead = eepromReadBuffer(address, unBuf.buffer, sizeof(unBuf));

    if (bytesRead == sizeof(unBuf)) {
        // Присваиваем считанные значения рабочим переменным
        settings.sp_structs[0].spT = unBuf.spDay.spT0;
        settings.sp_structs[1].spT = unBuf.spDay.spT1;
        settings.sp_structs[1].spRH = unBuf.spDay.spRH;
        settings.sp_structs[0].timer = unBuf.spDay.timer0;
        settings.sp_structs[1].timer = unBuf.spDay.timer1;
        settings.sp_structs[0].aeration = unBuf.spDay.aeration0;
        settings.sp_structs[1].aeration = unBuf.spDay.aeration1;
        settings.sp_structs[0].state = unBuf.spDay.flap;
        
        saveSetpoint(); // Сохранение в "/setpoint.json"
        
        // Переинициализация рабочих таймеров, чтобы изменения вступили в силу немедленно
        pvTimer = settings.sp_structs[0].timer; 
        pvAeration = settings.sp_structs[0].aeration;

        MYDEBUG_PRINTLN("-----------------------------------------");
        DEBUG_PRINTF("applyDailyProgram: ЗАГРУЖЕНА Программа %d, День %d (Адрес: 0x%04X)\n", prg, day, address);
        DEBUG_PRINTF("  Уставки T: %d, T: %d, Волог: %d\n", unBuf.spDay.spT0, unBuf.spDay.spT1, unBuf.spDay.spRH);
        DEBUG_PRINTF("  Таймеры(лотков): %d/%d, Провітрювання: %d/%d, Заслінка: %d\n", 
                     unBuf.spDay.timer0, unBuf.spDay.timer1, 
                     unBuf.spDay.aeration0, unBuf.spDay.aeration1, 
                     unBuf.spDay.flap);
        MYDEBUG_PRINTLN("-----------------------------------------");
    } else {
        MYDEBUG_PRINTLN("applyDailyProgram: ОШИБКА чтения из EEPROM!");
    }
}

// ----- Функция для подготовки стандартной талицы ----------
void prepareTable(uint8_t prg, uint8_t day, uint8_t amountday, int16_t t0, int16_t t1, 
  int16_t rh, int16_t timer, int16_t aer0, int16_t aer1, int16_t fl){
    
    unBuf.spDay.spT0 = t0;
    unBuf.spDay.spT1 = t1;
    unBuf.spDay.spRH = rh;
    unBuf.spDay.timer0 = timer;
    unBuf.spDay.timer1 = 0;
    unBuf.spDay.aeration0 = aer0;
    unBuf.spDay.aeration1 = aer1;
    unBuf.spDay.flap = fl;
    
    for (size_t i = 0; i < amountday; i++){
        uint8_t curday = day + i;
        uint16_t memoryAddress = eepromMemoryAddressForDay(prg, curday);
        byte res = eepromWriteBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
        MYDEBUG_PRINT("DAY:"); MYDEBUG_PRINT(curday); 
        MYDEBUG_PRINT("; ADD:"); MYDEBUG_PRINT(memoryAddress);
        MYDEBUG_PRINT("; RES:"); MYDEBUG_PRINTLN(res);
    }
    
}
// Інкубація курячих яєць. 
void prepareProg1(){
    MYDEBUG_PRINTLN("PROGRAMM: 1");
    prepareTable(1, 1, 5,379,310,60,60, 0, 0, 0);
    prepareTable(1, 6, 7,378,300,58,60,10,10,10);
    prepareTable(1,13, 6,376,290,55,60,10,10,20);
    prepareTable(1,19, 4,374,320,65, 0,10,10,30);
    prepareTable(1,23, 9,280,200,45, 0, 0, 0,90); // 23-31 (9 дней)
}

void prepareProg2(){
    MYDEBUG_PRINTLN("PROGRAMM: 2");
    prepareTable(2, 1, 8,380,330,70,60, 0, 0, 0);
    prepareTable(2, 9, 5,375,310,60,60,10,10,15);
    prepareTable(2,14,11,372,300,56,60,10,10,25);
    prepareTable(2,25, 4,370,320,70, 0,10,10,40);
    prepareTable(2,29, 3,280,200,45, 0, 0, 0,90); // 29-31 (3 дня)
}

void prepareProg3(){
    MYDEBUG_PRINTLN("PROGRAMM: 3");
    prepareTable(3, 1,12,376,310,58,60, 0, 0, 0);
    prepareTable(3,13, 3,373,290,53,60,10,10, 5);
    prepareTable(3,16, 2,372,280,47, 0,10,10,15);
    prepareTable(3,18, 2,370,340,80, 0,10,10,20);
    prepareTable(3,20,12,280,200,45, 0, 0, 0,90); // 20-31 (12 дней)
}

void prepareProg4(){
    MYDEBUG_PRINTLN("PROGRAMM: 4");
    prepareTable(4, 1, 6,378,300,56,60, 0, 0, 0);
    prepareTable(4, 7, 6,375,290,52,60,10,10,15);
    prepareTable(4,13,14,372,288,52,60,10,10,25);
    prepareTable(4,27, 2,370,320,70, 0,10,10,35);
    prepareTable(4,29, 3,280,200,45, 0, 0, 0,90); // 29-31 (3 дня)
}

uint8_t testProgs(){
  uint8_t err = 0;
  MYDEBUG_PRINTLN("AT24C32 EEPROM Test.");
  
  // 1. Проверка на "зеркальное" чтение (wrap-around)
  uint8_t buf0[8], buf700[8];
  eepromReadBuffer(0x000, buf0, 8);
  eepromReadBuffer(0x700, buf700, 8);
  
  bool aliasing = true;
  for(int i=0; i<8; i++) {
    if(buf0[i] != buf700[i]) aliasing = false;
  }
  
  if(aliasing && buf0[0] != 0xFF) {
    MYDEBUG_PRINTLN("!!! CRITICAL: EEPROM Aliasing detected! Addr 0x700 mirrors 0x000.");
  }

  // 2. Проверка и инициализация программ
  for (uint8_t p = 1; p <= 4; p++) {
      uint16_t memoryAddress = eepromMemoryAddressForDay(p, 1);
      eepromReadBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
      
      bool needRewrite = false;
      if (unBuf.spDay.spT0 == -1 || unBuf.spDay.spT0 == 0) needRewrite = true;
      if (unBuf.spDay.spT0 > 100 && unBuf.spDay.spT0 < 550 && (unBuf.spDay.flap < 0 || unBuf.spDay.flap > 100)) {
          needRewrite = true;
      }

      if (needRewrite) {
          MYDEBUG_PRINT("PROGRAMM N"); MYDEBUG_PRINT(p); MYDEBUG_PRINTLN(" invalid. Re-writing...");
          if(p == 1) prepareProg1();
          else if(p == 2) prepareProg2();
          else if(p == 3) prepareProg3();
          else if(p == 4) prepareProg4();
          err |= (1 << (p-1));
      } else {
          MYDEBUG_PRINT("PROGRAMM N"); MYDEBUG_PRINT(p); MYDEBUG_PRINTLN(" Ok");
      }
  }
  return err;
}
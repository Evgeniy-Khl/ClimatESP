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
// Інкубація курячих яєць. 2 дня в одной странице, 31 день ВСНГО 496 байт = 15 страниц
void prepareProg1(){
    MYDEBUG_PRINTLN("PROGRAMM: 1");
    prepareTable(1, 1, 5,379,310,60,60, 0, 0, 0);// 1-5	37,9 оС	31,0 оС(60,0%)	закрита	увімкнено	вимкнуто
    prepareTable(1, 6, 7,378,300,58,60,10,10,10);// 6-12	37,8 оС	30,0 оС(58,0%)	10 %	увімкнено	вимкнуто
    prepareTable(1,13, 6,376,290,55,60,10,10,20);// 13-18	37,6 оС	29,0 оС(55,0%)	20%	увімкнено	вимкнуто
    prepareTable(1,19, 4,374,320,65, 0,10,10,30);// 19-22	37,4 оС	32,0 оС(65,0%)	30%	вимкнений	вимкнуто
    prepareTable(1,23, 8,280,200,45, 0, 0, 0,90);// 23-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

// Інкубація качиних яєць.
void prepareProg2(){
    MYDEBUG_PRINTLN("PROGRAMM: 2");
    prepareTable(2, 1, 8,380,330,70,60, 0, 0, 0);// 1-8	38,0 оС	33,0 оС(70,0%)	закрита	увімкнено	вимкнуто
    prepareTable(2, 9, 5,375,310,60,60,10,10,15);// 9-13	37,5 оС	31,0 оС(60,0%)	15%	увімкнено	1 раз 5 хв.
    prepareTable(2,14,11,372,300,56,60,10,10,25);// 14-24	37,2 оС	30,0 оС(56,0%)	25%	увімкнено	2 рази 20 хв.
    prepareTable(2,25, 4,370,320,70, 0,10,10,40);// 25-28	37,0 оС	32,0 оС(70,0%)	40%	вимкнений	1 раз 10 хв.
    prepareTable(2,29, 2,280,200,45, 0, 0, 0,90);// 29-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

// Інкубація перепелиних яєць.
void prepareProg3(){
    MYDEBUG_PRINTLN("PROGRAMM: 3");
    prepareTable(3, 1,12,376,310,58,60, 0, 0, 0);// 1-12	37,6 оС	31,0 оС(58,0%)	закрита	увімкнено	вимкнуто
    prepareTable(3,13, 3,373,290,53,60,10,10, 5);// 13-15	37,3 оС	29,0 оС(53,0%)	5 %	увімкнено	вимкнуто
    prepareTable(3,16, 2,372,280,47, 0,10,10,15);// 16-17	37,2 оС	28,0 оС(47,0%)	15%	вимкнений	вимкнуто
    prepareTable(3,18, 2,370,340,80, 0,10,10,20);// 18-19	37,0 оС	34,0 оС(80,0%)	20%	вимкнений	вимкнуто
    prepareTable(3,20,11,280,200,45, 0, 0, 0,90);// 20-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

// Інкубація індикових яєць.
void prepareProg4(){
    MYDEBUG_PRINTLN("PROGRAMM: 4");
    prepareTable(4, 1, 6,378,300,56,60, 0, 0, 0);// 1-6	37,8 оС	30,0 оС(56,0%)	закрита	увімкнено	вимкнуто
    prepareTable(4, 7, 6,375,290,52,60,10,10,15);// 7-12	37,5 оС	29,0 оС(52,0%)	15 %	увімкнено	вимкнуто
    prepareTable(4,13,14,372,288,52,60,10,10,25);// 13-26	37,2 оС	28,8 оС(52,0%)	25%	увімкнено	вимкнуто
    prepareTable(4,27, 2,370,320,70, 0,10,10,35);// 27-28	37,0 оС	32,0 оС(70,0%)	35%	вимкнений	вимкнуто
    prepareTable(4,29, 2,280,200,45, 0, 0, 0,90);// 29-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

uint8_t testProgs(){
  uint8_t err = 0;
  MYDEBUG_PRINTLN("AT24C32 EEPROM Test.");
  uint16_t memoryAddress = eepromMemoryAddressForDay(1, 1);
  uint16_t res = eepromReadBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(res < sizeof(unBuf)) err = 1;
  if(unBuf.spDay.spT0 == -1){
    prepareProg1();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N1");
  } else MYDEBUG_PRINTLN("PROGRAMM N1 Ok");

  memoryAddress = eepromMemoryAddressForDay(2, 1);
  res = eepromReadBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(res < sizeof(unBuf)) err |= 2;
  if(unBuf.spDay.spT0 == -1){
    prepareProg2();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N2");
  } else MYDEBUG_PRINTLN("PROGRAMM N2 Ok");

  memoryAddress = eepromMemoryAddressForDay(3, 1);
  res = eepromReadBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(res < sizeof(unBuf)) err |= 4;
  if(unBuf.spDay.spT0 == -1){
    prepareProg3();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N3");
  } else MYDEBUG_PRINTLN("PROGRAMM N3 Ok");

  memoryAddress = eepromMemoryAddressForDay(4, 1);
  res = eepromReadBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(res < sizeof(unBuf)) err |= 8;
  if(unBuf.spDay.spT0 == -1){
    prepareProg4();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N4");
  } else MYDEBUG_PRINTLN("PROGRAMM N4 Ok");
  return err;
}
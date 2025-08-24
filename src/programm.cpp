#include "programm.h"

TableBuff unBuf;

/**
 * AT24C32 имеет страницы по 32 байта. 
 * адреса страниц: 0-31, 32-63, ... 
 * 0xF00 - 0xFFF резерв 255 байт.
 */
uint16_t eepromMemoryAddressForDay(uint8_t prg, uint8_t day){
	// из расчета одна страница на 1 день, 1 прог.=30 страниц всего 30*4=120 страниц плюс 8 страниц резерв.
    uint16_t addressPage = (day - 1) * 32 + (prg - 1) * (32*30);  
	return addressPage;
}

byte eepromWrBuff(uint16_t memoryAddress, const uint8_t* buffer, uint8_t length) {
  uint8_t currentBufferIndex = 0;
 
    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((uint8_t)(memoryAddress >> 8));   // Старший байт адреса
    Wire.write((uint8_t)(memoryAddress & 0xFF)); // Младший байт адреса

    for (uint8_t i = 0; i < length; ++i) {
      Wire.write(buffer[currentBufferIndex + i]);
    }

    byte status = Wire.endTransmission();
    if (status != 0) {
      MYDEBUG_PRINT("I2C Write Error in buffer (addr "); MYDEBUG_PRINT(memoryAddress);
      MYDEBUG_PRINT("). Status: "); MYDEBUG_PRINTLN(status);
      // Прервать дальнейшую запись этого буфера, если есть ошибка
    }
    delay(EEPROM_WRITE_DELAY); // Ожидание завершения цикла записи страницы
    return status; 
}

/**
 * @brief Читает массив байт (буфер) из EEPROM.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на буфер для сохранения прочитанных данных.
 * @param length Количество байт для чтения.
 */
void eepromRdBuff(uint16_t memoryAddress, uint8_t* buffer, uint8_t length) {
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));
  Wire.write((uint8_t)(memoryAddress & 0xFF));
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_I2C_ADDRESS, (int)length); // Запросить 'length' байт
  for (uint16_t i = 0; i < length; i++) {
    if (Wire.available()) {
      buffer[i] = Wire.read();
    } else {
      buffer[i] = 0; // В случае ошибки заполнить нулем
    }
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
        byte res = eepromWrBuff(memoryAddress, unBuf.buffer, sizeof(unBuf));
        MYDEBUG_PRINT("DAY:"); MYDEBUG_PRINT(curday); 
        MYDEBUG_PRINT("; ADD:"); MYDEBUG_PRINT(memoryAddress);
        MYDEBUG_PRINT("; RES:"); MYDEBUG_PRINTLN(res);
    }
    
}
// Інкубація курячих яєць. 2 дня в одной странице, 31 день ВСНГО 496 байт = 15 страниц
void prepareProg1(){
    MYDEBUG_PRINTLN("PROGRAMM: 1");
    prepareTable(1, 1, 5,379,310,600,60, 0, 0, 0);// 1-5	37,9 оС	31,0 оС(60,0%)	закрита	увімкнено	вимкнуто
    prepareTable(1, 6, 7,378,300,580,60,10,10,10);// 6-12	37,8 оС	30,0 оС(58,0%)	10 %	увімкнено	вимкнуто
    prepareTable(1,13, 6,376,290,550,60,10,10,20);// 13-18	37,6 оС	29,0 оС(55,0%)	20%	увімкнено	вимкнуто
    prepareTable(1,19, 4,374,320,650, 0,10,10,30);// 19-22	37,4 оС	32,0 оС(65,0%)	30%	вимкнений	вимкнуто
    prepareTable(1,23, 8,280,200,450, 0, 0, 0,90);// 23-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

// Інкубація качиних яєць.
void prepareProg2(){
    MYDEBUG_PRINTLN("PROGRAMM: 2");
    prepareTable(2, 1, 8,380,330,700,60, 0, 0, 0);// 1-8	38,0 оС	33,0 оС(70,0%)	закрита	увімкнено	вимкнуто
    prepareTable(2, 9, 5,375,310,600,60,10,10,15);// 9-13	37,5 оС	31,0 оС(60,0%)	15%	увімкнено	1 раз 5 хв.
    prepareTable(2,14,11,372,300,560,60,10,10,25);// 14-24	37,2 оС	30,0 оС(56,0%)	25%	увімкнено	2 рази 20 хв.
    prepareTable(2,25, 4,370,320,700, 0,10,10,40);// 25-28	37,0 оС	32,0 оС(70,0%)	40%	вимкнений	1 раз 10 хв.
    prepareTable(2,29, 2,280,200,450, 0, 0, 0,90);// 29-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

// Інкубація перепелиних яєць.
void prepareProg3(){
    MYDEBUG_PRINTLN("PROGRAMM: 3");
    prepareTable(3, 1,12,376,310,580,60, 0, 0, 0);// 1-12	37,6 оС	31,0 оС(58,0%)	закрита	увімкнено	вимкнуто
    prepareTable(3,13, 3,373,290,530,60,10,10, 5);// 13-15	37,3 оС	29,0 оС(53,0%)	5 %	увімкнено	вимкнуто
    prepareTable(3,16, 2,372,280,470, 0,10,10,15);// 16-17	37,2 оС	28,0 оС(47,0%)	15%	вимкнений	вимкнуто
    prepareTable(3,18, 2,370,340,800, 0,10,10,20);// 18-19	37,0 оС	34,0 оС(80,0%)	20%	вимкнений	вимкнуто
    prepareTable(3,20,11,280,200,450, 0, 0, 0,90);// 20-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

// Інкубація індикових яєць.
void prepareProg4(){
    MYDEBUG_PRINTLN("PROGRAMM: 4");
    prepareTable(4, 1, 6,378,300,560,60, 0, 0, 0);// 1-6	37,8 оС	30,0 оС(56,0%)	закрита	увімкнено	вимкнуто
    prepareTable(4, 7, 6,375,290,520,60,10,10,15);// 7-12	37,5 оС	29,0 оС(52,0%)	15 %	увімкнено	вимкнуто
    prepareTable(4,13,14,372,288,520,60,10,10,25);// 13-26	37,2 оС	28,8 оС(52,0%)	25%	увімкнено	вимкнуто
    prepareTable(4,27, 2,370,320,700, 0,10,10,35);// 27-28	37,0 оС	32,0 оС(70,0%)	35%	вимкнений	вимкнуто
    prepareTable(4,29, 2,280,200,450, 0, 0, 0,90);// 29-31	28,0 оС	20,0 оС(45,0%)	90%	вимкнений	вимкнуто
}

void testProgs(){
  MYDEBUG_PRINTLN("AT24C32 EEPROM Test.");
  uint16_t memoryAddress = eepromMemoryAddressForDay(1, 1);
  eepromRdBuff(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(unBuf.spDay.spT0 == -1){
    prepareProg1();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N1");
  } else MYDEBUG_PRINTLN("PROGRAMM N1 Ok");

  memoryAddress = eepromMemoryAddressForDay(2, 1);
  eepromRdBuff(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(unBuf.spDay.spT0 == -1){
    prepareProg2();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N2");
  } else MYDEBUG_PRINTLN("PROGRAMM N2 Ok");

  memoryAddress = eepromMemoryAddressForDay(3, 1);
  eepromRdBuff(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(unBuf.spDay.spT0 == -1){
    prepareProg3();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N3");
  } else MYDEBUG_PRINTLN("PROGRAMM N3 Ok");

  memoryAddress = eepromMemoryAddressForDay(4, 1);
  eepromRdBuff(memoryAddress, unBuf.buffer, sizeof(unBuf));
  if(unBuf.spDay.spT0 == -1){
    prepareProg4();
    MYDEBUG_PRINTLN("ПЕРЕЗАПИСАНА PROG N4");
  } else MYDEBUG_PRINTLN("PROGRAMM N4 Ok");
}
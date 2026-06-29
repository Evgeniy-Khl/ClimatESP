#include "main.h"
#include "my_settings.h"
char displStr[200];

ESP8266WebServer server(80);
WiFiClientSecure client;
MyTelegramBot bot(botToken, client);

PIDController pid[2];
SoftwarePWMBit heaterPwm(&portOut.value, 1); 
SoftwarePWMBit humidiPwm(&portOut.value, 2);

RTC_DS3231 rtc;                     // Создаем объект RTC для DS3231
DateTime now;

DHT dht(ONE_WIRE_BUS_PIN, DHT22);
OneWire oneWire(ONE_WIRE_BUS_PIN);  // Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
DallasTemperature sensors(&oneWire);// Передаем ссылку на объект oneWire в конструктор DallasTemperature
DeviceAddress sensorAddresses[MAX_DEVICE];  // Массив для хранения уникальных адресов датчиков

byte writePCF8574(byte data);
bool recoverI2C();

TM1638 module(13, 14, 12);    // Создаем объект module для TM1638
InvertedServo incubatorServo; // Создаем объект управления сервоприводом
void ledSet(void);

void setup(){
  logEvent("Система запускається...");
  #ifdef DEBUG
    Serial.begin(115200);               // Инициализация последовательного порта для отладки
  #endif

  ESP.wdtEnable(5000);                  // Включаем аппаратный ватчдог на 5 секунд

  Wire.begin();                         // Инициализация I2C (SDA, SCL по умолчанию для ESP8266 - GPIO4, GPIO5)
  Wire.setClock(100000);                // Снижаем скорость до 100кГц для стабильности
  Wire.setClockStretchLimit(150000);    // 150мс лимит clock stretch (защита от зависания)

  // --- I2C сканер: показывает все устройства на шине при старте ---
  {
    MYDEBUG_PRINTLN("\n--- I2C Bus Scan ---");
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      uint8_t err = Wire.endTransmission();
      if (err == 0) {
        char buf[48];
        snprintf(buf, sizeof(buf), "I2C found: 0x%02X", addr);
        MYDEBUG_PRINTLN(buf);
        logEvent(buf);
        found++;
      }
      ESP.wdtFeed();
    }
    if (found == 0) {
      MYDEBUG_PRINTLN("I2C scan: NO devices found!");
      logEvent("I2C scan: НЕТ устройств на шине!");
    }
    char buf[40];
    snprintf(buf, sizeof(buf), "I2C scan done. Devices: %d", found);
    MYDEBUG_PRINTLN(buf);
    MYDEBUG_PRINTLN("--- End of I2C scan ---\n");
  }


  // Первоначальный запуск RTC для корректного логирования времени с самого старта
  if (rtc.begin()) {
    RTCENABLE = 1;
    now = rtc.now();
  }

  /* uint8_t temp =  */writePCF8574(0xFF);    // Установить все пины в LOW (если они используются как выходы)

  // for (uint8_t i = 0; i < 8; i++) { data[i] = OO;}
  // module.setDisplay(data, 8);                       //"ooo ooo oo"
  // if(temp){
  //    dataLed[5] = 1;                                // ошибка writePCF8574
  // }
  //----------------------------------- MOUNTING FS ----------------------------------------
  MYDEBUG_PRINTLN("\n mounting FS...");
  bool lFS = LittleFS.begin();
  if(lFS) {
    fsMounted = true;
    MYDEBUG_PRINTLN("mounted file system");
    logEvent("Файлова система LittleFS змонтована успішно.");
    listFilesAndSizes();
    
    //--------------------- checkSetpoint ----------------------------------
    dataLed[2] = checkSetpoint();
    dataLed[3] = checkConfig();
  } else {
    MYDEBUG_PRINTLN("failed to mount FS");
    logEvent("ПОМИЛКА: Не вдалося змонтувати файлову систему LittleFS!");
    dataLed[4] = 1;
  }
  //---------------------------- инициализация WiFiManager -----------------------------------
  if(settings.sp_structs[0].special & 0x03) {
    ESP.wdtDisable();
    initWiFiManag();
    ESP.wdtEnable(5000);
  }
  else MYDEBUG_PRINTLN("Запрет на подключение к WiFi! Продолжаем работу в оффлайн-режиме.");
  //------------------------------------------------------------------------------
  PID_Init(&pid[0], settings.sp_structs[0].Kp, settings.sp_structs[0].Ki);
  PID_Init(&pid[1], settings.sp_structs[1].Kp, settings.sp_structs[1].Ki);
  //---------- Инициализация DS3231 ----------------------------------------
  if(RTCENABLE){
    logEvent("Годинник RTC DS3231 ініціалізовано успішно.");
    if(rtc.lostPower()) {                     // у RTC села батарейка
        logEvent("Увага: RTC втратив живлення! Час обнулено.");
        MYDEBUG_PRINTLN("RTC lost power! Текущая программа обнулена!");
        dataLed[1] = 1;                       // RTC lost power
        // Установка времени: 1 год, 1 месяц, 1 день, 00:00:00
        rtc.adjust(DateTime(2026, 1, 1, 0, 0, 0));
        settings.sp_structs[1].state = 0;     // [1]-программа текущая обнулена
        // saveSetpoint();  //????????????????????
    } else {
        now = rtc.now();                        // Инициализируем глобальное время перед восстановлением
        restoreIncubationStatus();              // восстановим флаг и время
    }
  } else {
    logEvent("ПОМИЛКА: Годинник RTC DS3231 не знайдено!");
    dataLed[0] = 1;      // DS3231 не инициализирован
  }
  //------------------------------------------------------------------------------
  testProgs();              // тест
  //----------------------- определяем какой датчик подключен --------------------------------
  sensorType();
  //------------------------------------------------------------------------------------------
  digitalWrite(BEEP_PIN, HIGH); // Выключаем бипер
  pinMode(BEEP_PIN, OUTPUT);    // Настраиваем пин бипера как выход только для LED

  displErrors();       // ИНДИКАЦИЯ ОШИБОК

  // Инициализируем сервопривод на GPIO15.
  // Стандартные параметры ширины импульса для большинства сервоприводов: 544 мкс (0°) - 2400 мкс (180°).
  incubatorServo.attach(15);
  pvFlap = settings.sp_structs[0].flapLimit;  // полностью закрыта 
  incubatorServo.write(pvFlap);
  
  pvTimer = settings.sp_structs[0].timer;                   // инициализация времени выключенного состояния таймера
  pvAeration = settings.sp_structs[0].aeration;             // инициализация ПАУЗы ПРОВЕТРИВАНИЯ (минут)
  heaterPwm.write(heaterValue);
  humidiPwm.write(humidiValue);
  portOut.value = 0xFF;
  delay(3000);
  IncubationManager::init();
  previousHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
  DEBUG_PRINTF("Free heap size: %d\n", previousHeapSize);
}

void loop(){
  ESP.wdtFeed();
  handleWiFi();          // Фоновое управление WiFi и переподключение
  server.handleClient(); // Обработка входящих запросов
  //--------------------------- УПРАВЛЕНИЕ СИМИСТОРОМ 10 mSec. ---------------------------------
  long nowMillis = millis();
  if(nowMillis - counter10 > 10){
    counter10 = nowMillis;
    hasChanged = false;
    hasChanged |= heaterPwm.update();
    hasChanged |= humidiPwm.update();

    if(hasChanged) {
      writePCF8574(portOut.value);
      ledSet();
    }

    if(beepOn) beepOn--; else digitalWrite(BEEP_PIN, HIGH);   // Выключаем бипер

    if(settings.sp_structs[0].mode == 4 && --pvPulse == 0){   // импульсный режим увлажнения
      humidiValue = TRIACOFF;
      writePCF8574(portOut.value);
      ledSet();
    }

    keys = module.getButtons();
    if(keys == 0) {waitCheckKeyPad = MINWAIT; keyCount = 0;}  // если не удерживается ни одна кнопка то сброс времени ожидания.
  }
  //-------------------------------------------- КЛАВИАТУРА --------------------------------------
    if(nowMillis - counterWait > waitCheckKeyPad){
      counterWait = nowMillis;
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
  if(nowMillis - counter500 > 500){
    halfSecond++;
    counter500 = nowMillis; 

    if(resetDispl) --resetDispl;
    else if(numSetup) saveset();  // сохраняем установки
    else displNum = 0;            // возврат к главному дисплею

    if(numSetup == 0) ledDispl(displNum); 
    else display_setup();

    module.setDisplay(data, 8);
    ledSet();                     // светодиоды панели
    // writePCF8574(portOut.value);  // ??????????????
  }
  //================================ НОВАЯ СЕКУНДА =================================
  if(nowMillis - counter1000 > 1000){
    counter1000 = nowMillis;
    newSecond();                  // обновление показаний датчиков, расчет PID и управление реле
    heaterPwm.write(heaterValue);
    humidiPwm.write(humidiValue);
    writePCF8574(portOut.value);
    byte portState = readPCF8574();
    // Извлекаем состояние 6-го и 7-го выводов (нумерация битов с 0)
    // RESERVE  = (portState & (1 << 6)) != 0; // true — на P6 уровень HIGH (открыт), false — притянут к LOW
    OVERHEAT = (portState & (1 << 7)) == 0; // true — на P7 уровень LOW (активный), false — уровень HIGH
    OutStatusLed();               // для HTML страницы
    incubatorServo.write(pvFlap);
    // #ifndef DEBUG
      if (++countSeconds > 59) {
      newMinute();
      countSeconds = 0;
    }
    // #else
    //   if(++countSeconds > 0) newMinute(); // В режиме отладки - каждая секунда это минута
    // #endif
  }
  //************************************************ TELEGRAM *************************************************/
  if (nowMillis - lastSendTime > interval) {
    if(earlyMode != mode){
      // DEBUG_PRINTF("mode:%d; seconds:%d; All time:%ld; \n", mode, seconds, allTime);
      earlyMode = mode;
    }
    lastSendTime = nowMillis;
    // Serial.print("Free heap size: ");
    // MYDEBUG_PRINTLN(system_get_free_heap_size());

    if(seconds==0 && mode == READDEFAULT) {mode = READEEPROM; interval = INTERVAL_1000;}
    else if(tableData[0][0]==0 && settings.sp_structs[1].state) {mode = READPROG; interval = INTERVAL_1000; quarter = GET_PROG1;}
    seconds += interval/1000;
    allTime += interval/1000;
    tmrTelegramOff -= interval/1000;  // if you use HTML telegram does not work (5 min.)
    
    /* switch (mode){
      case READEEPROM: getData(GET_EEPROM); break;
      case READPROG:   getData(quarter); break;
      case SAVEEEPROM: if(++tmrResetMode > 60) tmrResetMode = 0; mode = READDEFAULT; interval = INTERVAL_4000; break;
      case SAVEPROG:   if(++tmrResetMode > 60) tmrResetMode = 0; mode = READDEFAULT; interval = INTERVAL_4000; break;
      default: getData(GET_VALUES); break;
    } */
  }
}
//----------------------------------- END loop() ----------------------------------------------------------------------

void ledSet(void){
    byte led = 0;
    if(!TURN)   led |= 1;
    if(!HEATER) led |= 2;
    if(!HUMIDI) led |= 4;
    if(!EXTRA1) led |= 8;
    if(!EXTRA2) led |= 0x10;
    if(!EXTRA3) led |= 0x20;
    if(FROZE)  led |= 0x40;
    if(RUNAWAY) led|= 0x80;
    for (uint8_t i = 0; i < 8; i++){
        module.setLED(led&1, i);
        led >>= 1;
    }
}

// Функция для восстановления шины I2C (если SDA или SCL зависли)
// Возвращает true если PCF8574 откликается на шине после восстановления
bool recoverI2C() {
  static unsigned long lastRecoveryTime = 0;
  if (millis() - lastRecoveryTime < 500) return false; // Ограничение частоты попыток
  lastRecoveryTime = millis();

  MYDEBUG_PRINTLN("Attempting I2C bus recovery...");

  const uint8_t sda = 4; 
  const uint8_t scl = 5;

  // 1. Переводим пины в INPUT_PULLUP
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(5);

  MYDEBUG_PRINT("Bus state before pulses: SDA=");
  MYDEBUG_PRINT(digitalRead(sda));
  MYDEBUG_PRINT(", SCL=");
  MYDEBUG_PRINTLN(digitalRead(scl));

  // 2. Генерируем до 20 тактов SCL чтобы вывести ведомых из состояния передачи
  //    Используем более медленную частоту (~5кГц) для надёжности
  pinMode(scl, OUTPUT);
  for (int i = 0; i < 20; i++) {
    digitalWrite(scl, LOW);
    delayMicroseconds(100);
    digitalWrite(scl, HIGH);
    delayMicroseconds(100);
    if (digitalRead(sda) == HIGH && i >= 8) {
       MYDEBUG_PRINT("SDA released after ");
       MYDEBUG_PRINT(i + 1);
       MYDEBUG_PRINTLN(" pulses.");
       break;
    }
  }

  // 3. Генерируем корректную последовательность STOP
  pinMode(sda, OUTPUT);
  digitalWrite(sda, LOW);
  delayMicroseconds(100);
  digitalWrite(scl, HIGH);
  delayMicroseconds(100);
  digitalWrite(sda, HIGH);   // STOP: SDA LOW→HIGH при SCL HIGH
  delayMicroseconds(100);

  // 4. Возвращаем пины под управление Wire
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(20); // ждём стабилизации линий

  Wire.begin(sda, scl);
  Wire.setClock(100000);
  Wire.setClockStretchLimit(150000); // 150мс - предотвращаем таймаут при clock stretch
  
  delay(50); // Увеличенная пауза - дать Wire и устройствам время стабилизироваться

  if (digitalRead(sda) == LOW || digitalRead(scl) == LOW) {
    MYDEBUG_PRINT("I2C recovery: FAILED (bus stuck). SDA=");
    MYDEBUG_PRINT(digitalRead(sda));
    MYDEBUG_PRINT(", SCL=");
    MYDEBUG_PRINTLN(digitalRead(scl));
    return false;
  }

  // 5. Проверяем наличие PCF8574 на шине (как при старте)
  Wire.beginTransmission(PCF8574_ADDRESS);
  uint8_t err = Wire.endTransmission();
  if (err == 0) {
    MYDEBUG_PRINTLN("I2C recovery: SUCCESS (PCF8574 responds)");
    return true;
  } else {
    MYDEBUG_PRINT("I2C recovery: bus OK, but PCF8574 not found. err=");
    MYDEBUG_PRINTLN(err);
    return false;
  }
}

void enterI2cCriticalError() {
  logEvent("КРИТИЧНА ПОМИЛКА I2C: Робота зупинена! Очікування перезапуску...");
  MYDEBUG_PRINTLN("\n!!! CRITICAL I2C ERROR: Rebooting in 5 seconds...");

  // Сигнализируем ошибкой (~5 сек), затем перезагружаем
  for (uint8_t n = 0; n < 5; n++) {
    digitalWrite(BEEP_PIN, LOW);
    module.setDisplay(PCF_ERROR, 8);
    delay(500);
    digitalWrite(BEEP_PIN, HIGH);
    for (uint8_t i = 0; i < 8; i++) { data[i] = 0; }
    module.setDisplay(data, 8);
    delay(500);
  }

  // Перед перезапуском явно освобождаем I2C шину через STOP-условие.
  // ESP.restart() — программный сброс: он НЕ сбрасывает периферию на шине.
  // Если PCF8574/DS3231/AT24C32 удерживает SDA в LOW (незавершённая транзакция),
  // после рестарта Wire.begin() снова не найдёт устройства → бесконечный цикл.
  // Явный STOP гарантирует что slave отпустит шину до перезагрузки.
  // Wire.end() не поддерживается ESP8266 — просто перехватываем пины напрямую.
  const uint8_t sda = 4;
  const uint8_t scl = 5;
  pinMode(scl, OUTPUT); digitalWrite(scl, HIGH);
  pinMode(sda, OUTPUT); digitalWrite(sda, LOW);
  delayMicroseconds(100);
  digitalWrite(scl, HIGH);
  delayMicroseconds(100);
  digitalWrite(sda, HIGH); // SDA LOW→HIGH при SCL HIGH = STOP condition
  delayMicroseconds(100);
  // Возвращаем пины в INPUT_PULLUP — шина свободна
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(10);
  MYDEBUG_PRINTLN("I2C STOP sent. Rebooting...");

  ESP.restart();
}

static uint8_t i2c_error_count = 0;        // Счётчик последовательных ошибок I2C
static uint8_t recovery_fail_count = 0;    // Счётчик подряд неудачных recoverI2C()
// Примечание: флаг pcf8574_available удалён намеренно.
// «Мягкий режим» без перезагрузки не позволяет восстановить работу реле.
// При сбое PCF8574 выполняется 3 попытки recoverI2C() с паузой 5 сек,
// после чего выполняется перезагрузка.

// Делает до maxRetries попыток recoverI2C() с паузой pauseMs между ними.
// Если устройство не отвечает — вызывает enterI2cCriticalError() → ESP.restart().
static void waitForPCF8574OrReboot(uint8_t maxRetries = 3,
                                   unsigned long pauseMs = 5000UL) {
  logEvent("УВАГА: PCF8574 недоступний! Спроба відновлення...");
  MYDEBUG_PRINTLN("PCF8574: starting recovery (3 attempts, 5s pause)...");

  for (uint8_t attempt = 1; attempt <= maxRetries; attempt++) {
    ESP.wdtFeed();
    // Пауза перед попыткой — дать шине время стабилизироваться
    unsigned long waitStart = millis();
    while (millis() - waitStart < pauseMs) {
      ESP.wdtFeed();
      delay(100);
    }
    DEBUG_PRINTF("PCF8574: recovery attempt %d / %d\n", attempt, maxRetries);
    if (recoverI2C()) {
      MYDEBUG_PRINTLN("PCF8574: recovered successfully!");
      logEvent("PCF8574 відновлено. Продовжуємо роботу.");
      i2c_error_count = 0;
      recovery_fail_count = 0;
      return; // восстановились — выходим
    }
  }

  // Все 3 попытки провалились — перезагружаемся
  logEvent("PCF8574 не відновлено після 3 спроб. Перезапуск!");
  enterI2cCriticalError();
}

// Функция для записи байта на PCF8574
byte writePCF8574(byte data) {
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(data);
  byte error = Wire.endTransmission();

  if (error) {
    i2c_error_count++;
    MYDEBUG_PRINT("\nError writing to PCF8574. Code: ");
    MYDEBUG_PRINT(error);
    MYDEBUG_PRINT(" | Fail count: ");
    MYDEBUG_PRINTLN(i2c_error_count);

    // Первая попытка быстрого восстановления
    bool recovered = recoverI2C();
    if (recovered) {
      recovery_fail_count = 0;
      delay(50);
      Wire.beginTransmission(PCF8574_ADDRESS);
      Wire.write(data);
      error = Wire.endTransmission();
      if (error == 0) {
        i2c_error_count = 0;
        MYDEBUG_PRINTLN("writePCF8574: retry after recovery OK");
        return error;
      }
    }

    // Быстрое восстановление не помогло — запускаем цикл ожидания с таймаутом
    recovery_fail_count++;
    MYDEBUG_PRINT("PCF8574: recovery failed. Consecutive failures: ");
    MYDEBUG_PRINTLN(recovery_fail_count);
    waitForPCF8574OrReboot(); // 2 мин попыток → перезагрузка если безуспешно

    // Если дошли сюда — recoverI2C() вернул true внутри waitForPCF8574OrReboot,
    // повторяем запись
    delay(50);
    Wire.beginTransmission(PCF8574_ADDRESS);
    Wire.write(data);
    error = Wire.endTransmission();
  } else {
    i2c_error_count = 0;
    recovery_fail_count = 0;
  }
  return error;
}

// Функция для чтения байта с PCF8574.
// При ошибке пробует восстановить шину. Если не удаётся — возвращает кеш.
// Чтение не вызывает перезагрузку (за это отвечает writePCF8574).
byte readPCF8574() {
  static byte lastKnownValue = 0xFF; // Кеш последнего успешного чтения

  uint8_t count = Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (uint8_t)1);
  if (count > 0) {
    i2c_error_count = 0;
    lastKnownValue = Wire.read();
    return lastKnownValue;
  }

  // --- Ошибка чтения ---
  i2c_error_count++;
  MYDEBUG_PRINT("\nError reading from PCF8574. Fail count: ");
  MYDEBUG_PRINTLN(i2c_error_count);

  recoverI2C();

  // Повторная попытка после recovery
  delay(50);
  count = Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (uint8_t)1);
  if (count > 0) {
    i2c_error_count = 0;
    lastKnownValue = Wire.read();
    MYDEBUG_PRINTLN("readPCF8574: retry after recovery OK");
    return lastKnownValue;
  }

  // Второй retry
  delay(10);
  count = Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (uint8_t)1);
  if (count > 0) {
    i2c_error_count = 0;
    lastKnownValue = Wire.read();
    MYDEBUG_PRINTLN("readPCF8574: 2nd retry OK");
    return lastKnownValue;
  }

  // Не удалось прочитать — возвращаем кеш.
  // Перезагрузка будет инициирована через writePCF8574 в следующем цикле.
  MYDEBUG_PRINTLN("readPCF8574: using cached value");
  return lastKnownValue;
}



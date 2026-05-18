// server.cpp
#include <main.h>
#include "server.h"

extern uint8_t  seconds, mode, quarter;
extern long lastSendTime;
extern int tableData[32][4], tmrTelegramOff;

void notFoundHandler() {
  server.send(404, "text/plain", "Not found");
}

void formatFloat(char* buf, size_t size, float val, bool brackets) {
    if (brackets) snprintf(buf, size, "[%.1f]", val);
    else snprintf(buf, size, "%.1f", val);
    for (int i = 0; buf[i]; i++) if (buf[i] == '.') buf[i] = ',';
}

void respondsValues() {
    tmrTelegramOff = 300;
    JsonDocument data;
    
    char model[64];
    snprintf_P(model, sizeof(model), PSTR("Клімат-5.25&nbsp;&nbsp;&nbsp;&nbsp;№ %d"), settings.sp_structs[1].special & 0x0F);
    data["model"] = model;
    
    char t0[16], t0s[16], t1[16], t1s[16], hum[16], hums[16];

    formatFloat(t0, sizeof(t0), (float)ds[0].pvT/10.0, false);
    formatFloat(t0s, sizeof(t0s), (float)settings.sp_structs[0].spT/10.0, true);
    data["temperature0"] = t0;
    data["settemp0"] = t0s;
    
    if(detectedSensor == SENSOR_DS18B20 && numberOfDevices > 1){
        formatFloat(t1, sizeof(t1), (float)ds[1].pvT/10.0, false);
        formatFloat(t1s, sizeof(t1s), (float)settings.sp_structs[1].spT/10.0, true);
        data["temperature1"] = t1;
        data["settemp1"] = t1s;

        if(pvRH == 255) {
            data["humidity"] = "не визначено"; 
            data["sethum"] = " ";
        } else {
            snprintf(hum, sizeof(hum), "%d", pvRH);
            data["humidity"] = hum; 
            data["sethum"] = "%";
        }
        if(HIH5030) {
            formatFloat(hums, sizeof(hums), (float)settings.sp_structs[1].spRH/10.0, false);
            data["sethum"] = hums;
        }
    } else if(detectedSensor == SENSOR_DHT22){
        formatFloat(t1, sizeof(t1), (float)ds[1].pvT/10.0, false);
        formatFloat(hums, sizeof(hums), (float)settings.sp_structs[1].spRH/10.0, false);
        data["humidity"] = t1; // Для DHT22 второй "температурный" канал это влажность
        data["sethum"] = hums;
    }
    
    const char* str;
    switch (settings.sp_structs[1].extendMode){
      case 1: str = "охолодження"; break;
      case 2: str = "осущення"; break;
      case 3: str = "охол. и осуш."; break;
      default: str = ""; break;
    }
    data["ventmode"] = str;

    switch (settings.sp_structs[0].extendMode){
      case 0: str = "сирена"; break;
      case 1: str = "відключення"; break;
      default: str = ""; break;
    }
    data["extmode"] = str;
    
    switch (settings.sp_structs[0].mode){
      case 0: str = "немає"; break;
      case 1: str = "канал №1"; break;
      case 2: str = "канал №2"; break;
      case 3: str = "№1 и №2"; break;
      case 4: str = "імпульс"; break;
      default: str = ""; break;
    }
    data["relaymode"] = str;

    switch (settings.sp_structs[1].mode){
      case 0: str = "немає"; break;
      case 1: str = "присутня"; break;
      case 2: str = "немає (реле)"; break;
      case 3: str = "присутня (реле)"; break;
      default: str = ""; break;
    }
    data["checkDry"] = str;
    
    char rotation[32];
    snprintf_P(rotation, sizeof(rotation), PSTR("%d %s"), pvTimer, TURNSECOND ? "сек." : "хвл.");
    data["rotation"] = rotation;

    char power[16];
    snprintf_P(power, sizeof(power), PSTR("%d%%"), pctHeater);
    data["power"] = power;
    
    data["error1"] = (bool)ERROR1;
    data["error2"] = (bool)ERROR2;
    data["error4"] = (bool)ERROR4;
    data["error8"] = (bool)ERROR8;
    data["overheat"] = (bool)OVERHEAT;
    
    char flap[16];
    snprintf_P(flap, sizeof(flap), PSTR("%d%%"), pvFlap);
    data["flap"] = flap;
    
    if(settings.sp_structs[1].state==0) data["program"] = "немає";
    else {
      char prgStr[16];
      snprintf_P(prgStr, sizeof(prgStr), PSTR("№%d"), settings.sp_structs[1].state);
      data["program"] = prgStr;
    }
    data["currDay"] = "1 доба";
    
    data["led0"] = dataLed[0] ? "OFF" : "ON" ;
    data["led1"] = dataLed[1] ? "OFF" : "ON" ;
    data["led2"] = dataLed[2] ? "OFF" : "ON" ;
    data["led3"] = dataLed[3] ? "OFF" : "ON" ;
    data["led4"] = dataLed[4] ? "OFF" : "ON" ;
    data["led5"] = dataLed[5] ? "OFF" : "ON" ;
    
    server.setContentLength(measureJson(data));
    server.send(200, "application/json", "");
    serializeJson(data, server.client());
}

void respondsEeprom(){
    JsonDocument doc;
        doc["spT0"] = settings.sp_structs[0].spT;
        doc["spT1"] = settings.sp_structs[1].spT;
        doc["spRH0"] = settings.sp_structs[0].spRH;
        doc["spRH1"] = settings.sp_structs[1].spRH;
        doc["ventMode"] = settings.sp_structs[1].extendMode;
        doc["extendMode"] = settings.sp_structs[0].extendMode;
        doc["relayMode"] = settings.sp_structs[0].mode;
        doc["checkDry"] = settings.sp_structs[1].mode;
        doc["rotate0"] = settings.sp_structs[0].timer;
        doc["rotate1"] = settings.sp_structs[1].timer;
        doc["program"] = settings.sp_structs[1].state;
        doc["alarm0"] = settings.sp_structs[0].alarm;
        doc["alarm1"] = settings.sp_structs[1].alarm;
        doc["coolOn0"] = settings.sp_structs[0].coolOn;
        doc["coolOff0"] = settings.sp_structs[0].coolOff;
        doc["coolOn1"] = settings.sp_structs[1].coolOn;
        doc["coolOff1"] = settings.sp_structs[1].coolOff;
        doc["extOn"] = settings.sp_structs[0].auxiliary;
        doc["extOff"] = settings.sp_structs[1].auxiliary;
        doc["minRun"] = settings.sp_structs[0].pulse;
        doc["period"] = settings.sp_structs[1].pulse;
        doc["air0"] = settings.sp_structs[0].aeration;
        doc["air1"] = settings.sp_structs[1].aeration;
        doc["flpClose"] = settings.sp_structs[0].flapLimit;
        doc["flpOpen"] = settings.sp_structs[1].flapLimit;
        doc["flpNow"] = settings.sp_structs[0].state;
        doc["pkoff0"] = settings.sp_structs[0].Kp;
        doc["pkoff1"] = settings.sp_structs[1].Kp;
        doc["ikoff0"] = settings.sp_structs[0].Ki;
        doc["ikoff1"] = settings.sp_structs[1].Ki;
        doc["identif"] = settings.sp_structs[1].special;
        doc["status"] = 1;

        server.setContentLength(measureJson(doc));
        server.send(200, "application/json", "");
        serializeJson(doc, server.client());

        DEBUG_PRINTF("SERVER responds to the client with EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
}

void acceptEeprom() {
  // Логирование всех параметров
  DEBUG_PRINTF("The SERVER has accepted settings.sp_structs[]: %d, %ld\n", seconds, millis() - lastSendTime);
  
  for (uint8_t i = 0; i < server.args(); i++) {
      String paramName = server.argName(i);
      String paramValue = server.arg(i);
      
      // Логирование параметров (раскомментируйте, если нужно)
      // DEBUG_PRINTF("Parameter: %s, Value: %s\n", paramName.c_str(), paramValue.c_str());
      
      if (paramName == "spT0") settings.sp_structs[0].spT = paramValue.toInt();
      else if (paramName == "spT1") settings.sp_structs[1].spT = paramValue.toInt();
      else if (paramName == "spRH0") settings.sp_structs[0].spRH = paramValue.toInt();
      else if (paramName == "spRH1") settings.sp_structs[1].spRH = paramValue.toInt();
      else if (paramName == "ventMode") settings.sp_structs[1].extendMode = paramValue.toInt();
      else if (paramName == "extendMode") settings.sp_structs[0].extendMode = paramValue.toInt();
      else if (paramName == "relayMode") settings.sp_structs[0].mode = paramValue.toInt();
      else if (paramName == "checkDry") settings.sp_structs[1].mode = paramValue.toInt();
      else if (paramName == "rotate0") settings.sp_structs[0].timer = paramValue.toInt();
      else if (paramName == "rotate1") settings.sp_structs[1].timer = paramValue.toInt();
      else if (paramName == "program") settings.sp_structs[1].state = paramValue.toInt();
      else if (paramName == "alarm0") settings.sp_structs[0].alarm = paramValue.toInt();
      else if (paramName == "alarm1") settings.sp_structs[1].alarm = paramValue.toInt();
      else if (paramName == "coolOn0") settings.sp_structs[0].coolOn = paramValue.toInt();
      else if (paramName == "coolOff0") settings.sp_structs[0].coolOff = paramValue.toInt();
      else if (paramName == "coolOn1") settings.sp_structs[1].coolOn = paramValue.toInt();
      else if (paramName == "coolOff1") settings.sp_structs[1].coolOff = paramValue.toInt();
      else if (paramName == "extOn") settings.sp_structs[0].auxiliary = paramValue.toInt();
      else if (paramName == "extOff") settings.sp_structs[1].auxiliary = paramValue.toInt();
      else if (paramName == "minRun") settings.sp_structs[0].pulse = paramValue.toInt();
      else if (paramName == "period") settings.sp_structs[1].pulse = paramValue.toInt();
      else if (paramName == "air0") settings.sp_structs[0].aeration = paramValue.toInt();
      else if (paramName == "air1") settings.sp_structs[1].aeration = paramValue.toInt();
      else if (paramName == "flpClose") settings.sp_structs[0].flapLimit = paramValue.toInt();
      else if (paramName == "flpOpen") settings.sp_structs[1].flapLimit = paramValue.toInt();
      else if (paramName == "flpNow") settings.sp_structs[0].state = paramValue.toInt();
      else if (paramName == "pkoff0") settings.sp_structs[0].Kp = paramValue.toInt();
      else if (paramName == "pkoff1") settings.sp_structs[1].Kp = paramValue.toInt();
      else if (paramName == "ikoff0") settings.sp_structs[0].Ki = paramValue.toInt();
      else if (paramName == "ikoff1") settings.sp_structs[1].Ki = paramValue.toInt();
      else if (paramName == "identif") settings.sp_structs[1].special = paramValue.toInt();
  }

  server.send(200); // Отправляем только статус 200

  saveSetpoint();
}

  uint8_t respondsProgram(){
    uint8_t err = 0;
    JsonDocument doc;
    mode = SAVEPROG; interval = INTERVAL_1000; quarter = SET_PROG4+1;
    uint8_t prg = settings.sp_structs[1].state;
    if(prg){
      for (int i = 1; i < 31; i++) {
          JsonArray row = doc.add<JsonArray>();
          uint16_t memoryAddress = eepromMemoryAddressForDay(prg, i);
          uint16_t res = eepromReadBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
          if(res < sizeof(unBuf)) err++;
          row.add(unBuf.spDay.spT0);
          row.add(unBuf.spDay.spT1);
          row.add(unBuf.spDay.spRH);
          row.add(unBuf.spDay.flap);
          row.add(unBuf.spDay.timer0);
          row.add(unBuf.spDay.timer1);
          row.add(unBuf.spDay.aeration0);
          row.add(unBuf.spDay.aeration1);
      }
      
      server.setContentLength(measureJson(doc));
      server.send(200, "application/json", "");
      serializeJson(doc, server.client());
      
      DEBUG_PRINTF("SERVER responds to the client PROGRAM DATA #: %d,%ld\n",seconds,millis()-lastSendTime);
    }
    return err;
  }

  //https://arduinojson.org/v7/assistant/#/step1
  void programDeser(String input){
    uint8_t prg = settings.sp_structs[1].state;
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      MYDEBUG_PRINT("deserializeJson() FAILED: ");
      MYDEBUG_PRINTLN(error.c_str());
      return;
    }

    JsonArray data = doc["data"];
    MYDEBUG_PRINTLN("programDeser()");

    for (int i = 1; i < 31; i++) {
      JsonArray data_i = data[i];
      unBuf.spDay.spT0 = data_i[0]; //
      unBuf.spDay.spT1 = data_i[1]; //
      unBuf.spDay.spRH = data_i[2]; //
      unBuf.spDay.flap = data_i[3]; //
      unBuf.spDay.timer0 = data_i[4]; //
      unBuf.spDay.timer1 = data_i[5]; //
      unBuf.spDay.aeration0 = data_i[6]; //
      unBuf.spDay.aeration1 = data_i[7]; //
      
      MYDEBUG_PRINT("spT0="); MYDEBUG_PRINT(unBuf.spDay.spT0); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("spT1="); MYDEBUG_PRINT(unBuf.spDay.spT1); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("spRH="); MYDEBUG_PRINT(unBuf.spDay.spRH); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("flap="); MYDEBUG_PRINT(unBuf.spDay.flap); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("timer0="); MYDEBUG_PRINT(unBuf.spDay.timer0); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("timer1="); MYDEBUG_PRINT(unBuf.spDay.timer1); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("aeration0="); MYDEBUG_PRINT(unBuf.spDay.aeration0); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINT("aeration1="); MYDEBUG_PRINT(unBuf.spDay.aeration1); MYDEBUG_PRINT("; ");
      MYDEBUG_PRINTLN();
      uint16_t memoryAddress = eepromMemoryAddressForDay(prg, i);
      byte res = eepromWriteBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));

      MYDEBUG_PRINT("DAY:"); MYDEBUG_PRINT(i); 
      MYDEBUG_PRINT("; ADD:"); MYDEBUG_PRINT(memoryAddress);
      MYDEBUG_PRINT("; RES:"); MYDEBUG_PRINTLN(res);
    }
  }

  void acceptProgram() {
    String jsonData;

    // Проверка наличия параметра "data" в запросе
    if (server.hasArg("data")) {
        jsonData = server.arg("data");
        MYDEBUG_PRINTLN("jsonData: " + jsonData); // Логирование полученных данных
        
        // Отправляем статус 200
        server.send(200); 
        
        // Обработка полученных данных
        programDeser(jsonData);
        mode = SAVEPROG; interval = INTERVAL_1000;
        quarter = SET_PROG1;
        
        DEBUG_PRINTF("Accept Program: %d, %ld\n", seconds, millis() - lastSendTime);
    } else {
        // Отправка сообщения об ошибке, если параметр отсутствует
        server.send(400, "text/plain", "Ошибка: нет данных");
    }
  }

  /**
 * @brief Отправляет общую "шапку" HTML для всех страниц, включая CSS-стили.
 * @param title Заголовок страницы, который будет отображаться во вкладке браузера.
 */
void sendPageHeader(String title) {
    server.sendContent(F("<!DOCTYPE html><html><head><meta charset='utf-8'>"));
    server.sendContent(F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>"));
    server.sendContent("<title>" + title + "</title>");
    server.sendContent(F("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"));
    server.sendContent(F("<style>"));
    server.sendContent(F("body{font-family:Arial,sans-serif;background-color:#f4f4f4}"));
    server.sendContent(F("div{max-width:800px;margin:20px auto;padding:20px;background:#fff;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}"));
    server.sendContent(F("h1{text-align:center;color:#333}"));
    server.sendContent(F(".chart-container{position:relative;margin:auto;height:40vh;width:100%}"));
    server.sendContent(F("table{border-collapse:collapse;width:95%;margin:20px auto}"));
    server.sendContent(F("th,td{border:1px solid #ddd;text-align:center;padding:12px; font-size:1.1rem;}"));
    server.sendContent(F("th{background-color:#f2f2f2}tr:nth-child(even){background-color:#f9f9f9}"));
    server.sendContent(F("ul{list-style-type:none;padding:0}li{margin:15px 0}"));
    server.sendContent(F("a{display:block;padding:20px;background:#007bff;color:white;text-align:center;text-decoration:none;border-radius:8px;font-size:1.2rem;font-weight:bold;transition:background-color .3s}"));
    server.sendContent(F("a:hover{background-color:#0056b3}"));
    server.sendContent(F("a.back, a.btn{background-color:#6c757d; display:inline-block; padding:12px 24px; margin:20px 0;} a.back:hover, a.btn:hover{background-color:#5a6268}"));
    server.sendContent(F("a.live{background-color:#28a745}a.live:hover{background-color:#218838}"));
    server.sendContent(F(".summary{background-color:#eef; font-weight: bold;}"));
    server.sendContent(F("</style></head><body>"));
}

void handleGetGraph() {
    if (!server.hasArg("day") || server.arg("day") == "") {
        server.send(400, "text/plain", F("Bad Request: 'day' parameter is missing"));
        return;
    }
    String day = server.arg("day");
    String filename = "/day_" + day + "_graph.json";
    
    if (LittleFS.exists(filename)) {
        File file = LittleFS.open(filename, "r");
        server.streamFile(file, "application/json");
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

void handleGetCurrentGraph() {
    int currentPeriod = (countHours * 60 + countMinutes) / 5;
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (int period = 0; period <= currentPeriod; period++) {
        int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
        int16_t raw_t1, raw_t2, raw_rh;
        eepromReadInt16(currentAddress, raw_t1);
        eepromReadInt16(currentAddress + 2, raw_t2);
        eepromReadInt16(currentAddress + 4, raw_rh);

        if (raw_t1 == 0 && raw_t2 == 0) continue; 

        JsonObject point = array.add<JsonObject>();
        point["p"] = period;
        point["t1"] = (float)raw_t1 / 10.0;
        point["t2"] = (float)raw_t2 / 10.0;
        point["rh"] = (float)raw_rh;
    }
    
    server.setContentLength(measureJson(doc));
    server.send(200, "application/json", "");
    serializeJson(doc, server.client());
}

/**
 * @brief Генерирует страницу со списком дней в обратном порядке (новые вверху).
 */
void handleArchiveList() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    sendPageHeader("Інкубатор - Архів");

    server.sendContent(F("<div><h1>Виберіть добу для перегляду</h1>"));
    server.sendContent(F("<a href='/current' class='live' style='margin-bottom:20px;'>Перегляд ПОТОЧНОЇ доби</a>"));
    server.sendContent(F("<ul>"));
    
    // Шаг 1: Собираем все номера дней в вектор
    std::vector<int> days;
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        String fileName = dir.fileName();
        if (fileName.startsWith("day_") && fileName.endsWith("_graph.json")) {
            int start = 4;
            int end = fileName.indexOf('_', start);
            if (end > start) {
                // Преобразуем номер дня в число и добавляем в вектор
                days.push_back(fileName.substring(start, end).toInt());
            }
        }
    }

    // Шаг 2: Сортируем вектор по возрастанию (например, 1, 2, 10, 11)
    std::sort(days.begin(), days.end());

    // Шаг 3: Идем по отсортированному вектору в ОБРАТНОМ порядке и генерируем ссылки
    for (int i = days.size() - 1; i >= 0; i--) {
        int day = days[i];
        char link[128];
        snprintf_P(link, sizeof(link), PSTR("<li><a href='/data?day=%d'>Перегляд даних за %d добу</a></li>"), day, day + 1);
        server.sendContent(link);
        yield();
    }

    server.sendContent(F("</ul><div style='text-align:center;'><a href='/' class='back'>Назад на головну</a></div>"));
    server.sendContent(F("</div></body></html>"));
    server.sendContent(F(""));
}


void formatTimeBuffer(char* buf, size_t size, int period, int count) {
  int totalMinutes = period * 5;

  int hours = totalMinutes / 60;
  int minutes = totalMinutes % 60;
  snprintf_P(buf, size, PSTR("%02d:%02d"), hours, minutes);
}
/**
 * @brief Генерирует страницу с таблицей, отправляя ВЕСЬ HTML по частям.
 * Максимально экономный по памяти вариант.
 */
void handleShowData() {
    if (!server.hasArg("day") || server.arg("day") == "") {
        server.send(400, "text/plain", F("Bad Request: 'day' parameter is missing or empty"));
        return;
    }
    String day = server.arg("day");

    // 1. Читаем только файл статистики (он маленький)
    String statsFilename = "/day_" + day + "_stats.json";
    File statsFile = LittleFS.open(statsFilename, "r");
    JsonDocument statsDoc;
    if (statsFile) {
        deserializeJson(statsDoc, statsFile);
        statsFile.close();
    }

    // 2. Начинаем отправку HTML страницы
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    sendPageHeader("Інтубатор - Доба " + String(day.toInt() + 1));

    server.sendContent(F("<div><h1 style='text-align:center;'>Дані інкубації за "));
    server.sendContent(String(day.toInt() + 1)); 
    server.sendContent(F(" добу</h1>"));

    // --- Вставка графика (JS fetch сам заберет JSON файл через /get_graph) ---
    server.sendContent(F("<div class='chart-container'><canvas id='tempChart'></canvas></div>"));
    server.sendContent(F("<script>"));
    server.sendContent("const dayNum = " + day + ";");
    server.sendContent(F(R"raw(
    fetch('/get_graph?day=' + dayNum)
      .then(r => r.json())
      .then(data => {
        const labels = data.map(p => {
            let total = p.p * 5;
            let h = Math.floor(total / 60);
            let m = total % 60;
            return h.toString().padStart(2, '0') + ':' + m.toString().padStart(2, '0');
        });
        const t1 = data.map(p => p.t1);
        const t2 = data.map(p => p.t2);
        const rh = data.map(p => p.rh);
        new Chart(document.getElementById('tempChart'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [
              { label: 'T1 (°C)', data: t1, borderColor: '#ff4d4d', backgroundColor: '#ff4d4d', tension: 0.3, pointRadius: 2 },
              { label: 'T2 (°C)', data: t2, borderColor: '#28a745', backgroundColor: '#28a745', tension: 0.3, pointRadius: 2 },
              { label: 'Вологість (%)', data: rh, borderColor: '#3399ff', backgroundColor: '#3399ff', tension: 0.3, pointRadius: 2, yAxisID: 'y1' }
            ]
          },
          options: { 
            responsive: true, maintainAspectRatio: false,
            scales: { 
                y: { type: 'linear', display: true, position: 'left', title: { display: true, text: 'Темп. (°C)' } },
                y1: { type: 'linear', display: true, position: 'right', min: 0, max: 100, grid: { drawOnChartArea: false }, title: { display: true, text: 'Волог. (%)' } }
            },
            interaction: { intersect: false, mode: 'index' }
          }
        });
      });
    )raw"));
    server.sendContent(F("</script>"));

    server.sendContent(F("<div style='text-align:center;'><a href='/archive' class='btn'>Назад до списку діб</a></div>"));
    server.sendContent(F("<table>"));
    server.sendContent(F("<tr><th>Відлік часу від початку інкубації</th><th>Температура t1 (°C)</th><th>Температура t2 (°C)</th><th>Вологість (%)</th></tr>"));

    // 3. Вывод строки статистики
    if (!statsDoc.isNull()) {
        String summaryRow = "<tr><th>через кожні 5 хвилин</th><th>Середнє: ";
        summaryRow += String(statsDoc["avg_t1"].as<float>(), 1) + "<br>Min: " + String(statsDoc["min_t1"].as<float>(), 1) + "<br>Max: " + String(statsDoc["max_t1"].as<float>(), 1);
        summaryRow += "</th><th>Середнє: " + String(statsDoc["avg_t2"].as<float>(), 1) + "<br>Min: " + String(statsDoc["min_t2"].as<float>(), 1) + "<br>Max: " + String(statsDoc["max_t2"].as<float>(), 1);
        summaryRow += "</th><th>Середнє: " + String(statsDoc["avg_rh"].as<float>(), 1) + "<br>Min: " + String(statsDoc["min_rh"].as<float>(), 1) + "<br>Max: " + String(statsDoc["max_rh"].as<float>(), 1);
        summaryRow += "</th></tr>";
        server.sendContent(summaryRow);
    }
    
    // 4. Построчный вывод таблицы из файла графиков
    String graphFilename = "/day_" + day + "_graph.json";
    File graphFile = LittleFS.open(graphFilename, "r");
    if (graphFile) {
        // Мы не используем deserializeJson на весь файл!
        // Вместо этого мы используем ArduinoJson для парсинга отдельных объектов в массиве.
        // Для простоты в данном контексте: раз файл уже в JSON формате, мы можем 
        // прочитать его как поток и выводить строки.
        JsonDocument tempDoc;
        DeserializationError err = deserializeJson(tempDoc, graphFile);
        if (!err) {
           JsonArray array = tempDoc.as<JsonArray>();
           for (int i = array.size() - 1; i >= 0; i--) {
                JsonObject point = array[i];
                char row[128];
                char fmtTime[32];
                formatTimeBuffer(fmtTime, sizeof(fmtTime), point["p"].as<int>(), 287);
                snprintf_P(row, sizeof(row), PSTR("<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>"), 
                           fmtTime, point["t1"].as<float>(), point["t2"].as<float>(), point["rh"].as<float>());
                server.sendContent(row);
                if (i % 10 == 0) yield(); // Даем передышку Wi-Fi стеку
           }
        }
        graphFile.close();
    }

    server.sendContent(F("</table><div style='text-align:center;'><a href='/archive' class='btn'>Назад до списку діб</a></div>"));
    server.sendContent(F("</div></body></html>"));
    server.sendContent("");
}

/**
 * @brief Генерирует таблицу с данными за ТЕКУЩИЙ день, читая их напрямую из AT24C32.
 */
void handleCurrentData() {
    int currentPeriod = (countHours * 60 + countMinutes) / 5;

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    sendPageHeader("Інкубатор - Поточна доба");

    server.sendContent(F("<div><h1 style='text-align:center;'>Дані за поточну добу</h1>"));

    char timeStr[32];
    snprintf_P(timeStr, sizeof(timeStr), PSTR("<p style='text-align:center;'>Інформація оновлена в %02u:%02u</p>"), countHours, countMinutes);
    server.sendContent(timeStr);

    // --- Вставка графика для текущей суток ---
    server.sendContent(F("<div class='chart-container'><canvas id='tempChart'></canvas></div>"));
    server.sendContent(F("<script>"));
    server.sendContent(F(R"raw(
    fetch('/get_current_graph')
      .then(r => r.json())
      .then(data => {
        const labels = data.map(p => {
            let total = p.p * 5;
            let h = Math.floor(total / 60);
            let m = total % 60;
            return h.toString().padStart(2, '0') + ':' + m.toString().padStart(2, '0');
        });
        const t1 = data.map(p => p.t1);
        const t2 = data.map(p => p.t2);
        const rh = data.map(p => p.rh);
        new Chart(document.getElementById('tempChart'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [
              { label: 'T1 (°C)', data: t1, borderColor: '#ff4d4d', backgroundColor: '#ff4d4d', tension: 0.3, pointRadius: 2 },
              { label: 'T2 (°C)', data: t2, borderColor: '#28a745', backgroundColor: '#28a745', tension: 0.3, pointRadius: 2 },
              { label: 'Вологість (%)', data: rh, borderColor: '#3399ff', backgroundColor: '#3399ff', tension: 0.3, pointRadius: 2, yAxisID: 'y1' }
            ]
          },
          options: { 
            responsive: true, maintainAspectRatio: false,
            scales: { 
                y: { type: 'linear', display: true, position: 'left', title: { display: true, text: 'Темп. (°C)' } },
                y1: { type: 'linear', display: true, position: 'right', min: 0, max: 100, grid: { drawOnChartArea: false }, title: { display: true, text: 'Волог. (%)' } }
            },
            interaction: { intersect: false, mode: 'index' }
          }
        });
      });
    )raw"));
    server.sendContent(F("</script>"));

    server.sendContent(F("<div style='text-align:center;'><a href='/archive' class='btn'>Назад до архіву</a></div>"));
    server.sendContent(F("<table><tr><th>Відлік часу з моменту увімкнення приладу</th><th>T1 (°C)</th><th>T2 (°C)</th><th>RH (%)</th></tr>"));

    for (int period = currentPeriod; period >= 0; period--) {
        int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
        int16_t raw_t1, raw_t2, raw_rh;
        eepromReadInt16(currentAddress, raw_t1);
        eepromReadInt16(currentAddress + 2, raw_t2);
        eepromReadInt16(currentAddress + 4, raw_rh);

        if (raw_t1 == 0 && raw_t2 == 0) continue; 

        char row[128];
        char fmtTime[32];
        formatTimeBuffer(fmtTime, sizeof(fmtTime), period, currentPeriod);
        snprintf_P(row, sizeof(row), PSTR("<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>"), 
                   fmtTime, (float)raw_t1 / 10.0, (float)raw_t2 / 10.0, (float)raw_rh);

        server.sendContent(row);
        yield();
    }
    server.sendContent(F("</table><div style='text-align:center;'><a href='/archive' class='btn'>Назад до архіву</a></div>"));
    server.sendContent(F("</div></body></html>"));
}
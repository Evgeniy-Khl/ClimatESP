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
    
    // Инициализируем значениями по умолчанию во избежание undefined на веб-интерфейсе
    data["humidity"] = "--";
    data["sethum"] = " ";
    
    if(detectedSensor == SENSOR_DS18B20){
        if (numberOfDevices > 1) {
            formatFloat(t1, sizeof(t1), (float)ds[1].pvT/10.0, false);
            formatFloat(t1s, sizeof(t1s), (float)settings.sp_structs[1].spT/10.0, true);
            data["temperature1"] = t1;
            data["settemp1"] = t1s;
        }
        
        if (HIH5030 || numberOfDevices > 1) {
            if(pvRH == 255) {
                data["humidity"] = "не визначено"; 
                data["sethum"] = " ";
            } else {
                snprintf(hum, sizeof(hum), "%d", pvRH);
                data["humidity"] = hum; 
                data["sethum"] = " ";
            }
            if(HIH5030) {
                formatFloat(hums, sizeof(hums), (float)settings.sp_structs[1].spRH/10.0, true);
                data["sethum"] = hums;
            }
        }
    } else if(detectedSensor == SENSOR_DHT22){
        formatFloat(t1, sizeof(t1), (float)ds[1].pvT/10.0, false);
        formatFloat(hums, sizeof(hums), (float)settings.sp_structs[1].spRH/10.0, true); // Скобки вокруг уставки
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
    char currDay[16];
    snprintf_P(currDay, sizeof(currDay), PSTR("%d доба"), countDays + 1);
    data["currDay"] = currDay;

    // Время старта инкубации
    uint8_t start_data[7];
    if (eepromReadBuffer(INCUBATION_DATA_ADRES, start_data, 7) == 7 && start_data[0] > 0) {
      char startStr[20];
      snprintf_P(startStr, sizeof(startStr), PSTR("%02d.%02d.%02d %02d:%02d"), 
                 start_data[3], start_data[2], start_data[1], start_data[4], start_data[5]);
      data["startTime"] = startStr;
    } else {
      data["startTime"] = "---";
    }

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
        doc["botToken"] = botToken;
        doc["chatID"] = chatID;
        doc["status"] = 1;

        server.setContentLength(measureJson(doc));
        server.send(200, "application/json", "");
        serializeJson(doc, server.client());

        // DEBUG_PRINTF("SERVER responds to the client with EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
}

void acceptEeprom() {
  bool configChanged = false;

  // Запоминаем текущие значения до применения новых
  int16_t old_spT0   = settings.sp_structs[0].spT;
  int16_t old_spT1   = settings.sp_structs[1].spT;
  int16_t old_spRH   = settings.sp_structs[0].spRH;
  int16_t old_alarm0 = settings.sp_structs[0].alarm;
  int16_t old_alarm1 = settings.sp_structs[1].alarm;
  int16_t old_flpNow = settings.sp_structs[0].state;
  int16_t old_aux0    = settings.sp_structs[0].auxiliary;
  int16_t old_aux1    = settings.sp_structs[1].auxiliary;
  int16_t old_coolOn0 = settings.sp_structs[0].coolOn;
  int16_t old_coolOff0= settings.sp_structs[0].coolOff;
  int16_t old_coolOn1 = settings.sp_structs[1].coolOn;
  int16_t old_coolOff1= settings.sp_structs[1].coolOff;
  int16_t old_air0    = settings.sp_structs[0].aeration;
  int16_t old_air1    = settings.sp_structs[1].aeration;

  for (uint8_t i = 0; i < server.args(); i++) {
      String paramName = server.argName(i);
      String paramValue = server.arg(i);
      
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
      else if (paramName == "program") {
        int16_t newProg = paramValue.toInt();
        if (settings.sp_structs[1].state != newProg) {
          settings.sp_structs[1].state = newProg;
          startIncubation();
        }
      }
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
      else if (paramName == "botToken") {
          if (paramValue != botToken) {
              strncpy(botToken, paramValue.c_str(), sizeof(botToken) - 1);
              botToken[sizeof(botToken) - 1] = '\0';
              configChanged = true;
          }
      }
      else if (paramName == "chatID") {
          if (paramValue != chatID) {
              strncpy(chatID, paramValue.c_str(), sizeof(chatID) - 1);
              chatID[sizeof(chatID) - 1] = '\0';
              configChanged = true;
          }
      }
  }

  server.send(200);
  saveSetpoint();

  // Логируем только изменённые параметры
  String logStr = "Налаштування: ";
  bool anyChanged = false;
  char tmp[64];

  #define LOG_CHANGED_F(label, oldVal, newVal, divisor) \
    if ((oldVal) != (newVal)) { \
      if (anyChanged) logStr += " | "; \
      snprintf(tmp, sizeof(tmp), label "%.1f->%.1f", (oldVal)/(divisor), (newVal)/(divisor)); \
      logStr += tmp; anyChanged = true; \
    }
  #define LOG_CHANGED_D(label, oldVal, newVal) \
    if ((oldVal) != (newVal)) { \
      if (anyChanged) logStr += " | "; \
      snprintf(tmp, sizeof(tmp), label "%d->%d", (int)(oldVal), (int)(newVal)); \
      logStr += tmp; anyChanged = true; \
    }

  LOG_CHANGED_F("Температура №1:",  old_spT0,   settings.sp_structs[0].spT,   10.0f)
  LOG_CHANGED_F("Температура №2:",  old_spT1,   settings.sp_structs[1].spT,   10.0f)
  LOG_CHANGED_D("Вологість:",       old_spRH,   settings.sp_structs[0].spRH)
  LOG_CHANGED_F("Аварія №1:",       old_alarm0, settings.sp_structs[0].alarm, 10.0f)
  LOG_CHANGED_F("Аварія №2:",       old_alarm1, settings.sp_structs[1].alarm, 10.0f)
  LOG_CHANGED_D("Заслінка:",        old_flpNow,  settings.sp_structs[0].state)
  LOG_CHANGED_D("Увімкнення доп.:", old_aux0,   settings.sp_structs[0].auxiliary)
  LOG_CHANGED_D("Вимкнення доп.:",  old_aux1,   settings.sp_structs[1].auxiliary)
  LOG_CHANGED_D("Увімкнення охол.:",old_coolOn0, settings.sp_structs[0].coolOn)
  LOG_CHANGED_D("Вимкнення охол.:", old_coolOff0,settings.sp_structs[0].coolOff)
  LOG_CHANGED_D("Увімкнення осуш.:",old_coolOn1, settings.sp_structs[1].coolOn)
  LOG_CHANGED_D("Вимкнення осуш.:", old_coolOff1,settings.sp_structs[1].coolOff)
  LOG_CHANGED_D("Пауза провіт.:",   old_air0,    settings.sp_structs[0].aeration)
  LOG_CHANGED_D("Тривалість провіт.:", old_air1,    settings.sp_structs[1].aeration)

  #undef LOG_CHANGED_F
  #undef LOG_CHANGED_D

  if (anyChanged) logEvent(logStr.c_str());
//   DEBUG_PRINTF("The SERVER has accepted settings.sp_structs[]: %d, %ld\n", seconds, millis() - lastSendTime);

  if (configChanged) {
      JsonDocument json;
      json["botToken"] = botToken;
      json["chatID"] = chatID;
      File configFile = LittleFS.open("/config.json", "w");
      if (configFile) {
          serializeJson(json, configFile);
          configFile.close();
          MYDEBUG_PRINTLN("Saved new Telegram config to /config.json");
      }
      botStarted = false;
      BOTENABLE = 0;
      bot.updateToken(botToken);
  }
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
      
    //   DEBUG_PRINTF("SERVER responds to the client PROGRAM DATA #: %d,%ld\n",seconds,millis()-lastSendTime);
    }
    return err;
}

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
      unBuf.spDay.spT0 = data_i[0];
      unBuf.spDay.spT1 = data_i[1];
      unBuf.spDay.spRH = data_i[2];
      unBuf.spDay.flap = data_i[3];
      unBuf.spDay.timer0 = data_i[4];
      unBuf.spDay.timer1 = data_i[5];
      unBuf.spDay.aeration0 = data_i[6];
      unBuf.spDay.aeration1 = data_i[7];
      
      uint16_t memoryAddress = eepromMemoryAddressForDay(prg, i);
      eepromWriteBuffer(memoryAddress, unBuf.buffer, sizeof(unBuf));
    }
}

void acceptProgram() {
    if (server.hasArg("data")) {
        programDeser(server.arg("data"));
        server.send(200);
        mode = SAVEPROG; interval = INTERVAL_1000;
        quarter = SET_PROG1;
    } else {
        server.send(400, "text/plain", "Ошибка: нет данных");
    }
}

void sendPageHeader(String title) {
    server.sendContent(F("<!DOCTYPE html><html><head><meta charset='utf-8'>"));
    server.sendContent(F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>"));
    server.sendContent("<title>" + title + "</title>");
    server.sendContent(F("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"));
    server.sendContent(F("<style>"));
    server.sendContent(F("@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap');"));
    server.sendContent(F("body{font-family:'Inter',sans-serif;background:linear-gradient(135deg,#0f172a 0%,#1e293b 100%);color:#f8fafc;min-height:100vh;margin:0;padding:20px 10px;display:flex;flex-direction:column;align-items:center}"));
    server.sendContent(F("*{box-sizing:border-box}.container{max-width:800px;width:100%;margin:0 auto;padding:20px;background:rgba(255,255,255,0.06);backdrop-filter:blur(12px);-webkit-backdrop-filter:blur(12px);border:1px solid rgba(255,255,255,0.1);border-radius:16px;box-shadow:0 8px 32px 0 rgba(0,0,0,0.2)}"));
    server.sendContent(F("h1{text-align:center;color:#fff;margin-bottom:20px;font-weight:700}"));
    server.sendContent(F(".chart-container{position:relative;margin:20px auto;height:40vh;width:100%;background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.05);border-radius:12px;padding:10px}"));
    server.sendContent(F("table{border-collapse:collapse;width:100%;margin:20px auto;font-size:0.95rem;border-radius:12px;overflow:hidden;border:1px solid rgba(255,255,255,0.08)}"));
    server.sendContent(F("th,td{border:1px solid rgba(255,255,255,0.05);text-align:center;padding:12px}"));
    server.sendContent(F("th{background-color:rgba(15,23,42,0.5);color:#94a3b8;font-weight:600;text-transform:uppercase;font-size:0.75rem;letter-spacing:0.03em}"));
    server.sendContent(F("tr{transition:background-color .2s}tr:nth-child(even){background-color:rgba(255,255,255,0.01)}tr:hover{background-color:rgba(255,255,255,0.03)}"));
    server.sendContent(F("ul{list-style-type:none;padding:0;display:flex;flex-direction:column;gap:10px}li{margin:0}"));
    server.sendContent(F("a{display:block;padding:14px 20px;background:linear-gradient(135deg,#2563eb 0%,#1d4ed8 100%);color:white;text-align:center;text-decoration:none;border-radius:12px;font-size:1.05rem;font-weight:600;box-shadow:0 4px 12px rgba(37,99,235,0.2);transition:all 0.2s ease}"));
    server.sendContent(F("a:hover{transform:translateY(-2px);box-shadow:0 6px 16px rgba(37,99,235,0.35);background:linear-gradient(135deg,#3b82f6 0%,#2563eb 100%)}"));
    server.sendContent(F("a.back, a.btn{background:rgba(255,255,255,0.08);border:1px solid rgba(255,255,255,0.1);display:inline-block;padding:12px 24px;margin:20px 0;box-shadow:none}"));
    server.sendContent(F("a.back:hover, a.btn:hover{background:rgba(255,255,255,0.12);transform:translateY(-2px)}"));
    server.sendContent(F("a.live{background:linear-gradient(135deg,#10b981 0%,#059669 100%);box-shadow:0 4px 12px rgba(16,185,129,0.2)}a.live:hover{background:linear-gradient(135deg,#34d399 0%,#10b981 100%);box-shadow:0 6px 16px rgba(16,185,129,0.35)}"));
    server.sendContent(F(".summary{background-color:rgba(59,130,246,0.1);font-weight:bold;color:#60a5fa}"));
    server.sendContent(F(".archive-item{display:flex;justify-content:space-between;align-items:center;padding:12px 16px;background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.05);border-radius:12px;margin-bottom:8px}"));
    server.sendContent(F(".btn-group{display:flex;gap:8px}"));
    server.sendContent(F("a.btn-archive{display:inline-block;padding:8px 16px;font-size:0.85rem;border-radius:8px;box-shadow:none}"));
    server.sendContent(F("a.btn-archive:hover{transform:translateY(-1px)}"));
    server.sendContent(F("a.btn-graph{background:linear-gradient(135deg,#3b82f6 0%,#1d4ed8 100%)}"));
    server.sendContent(F("a.btn-graph:hover{background:linear-gradient(135deg,#60a5fa 0%,#2563eb 100%);box-shadow:0 4px 12px rgba(59,130,246,0.25)}"));
    server.sendContent(F("a.btn-logs{background:linear-gradient(135deg,#10b981 0%,#047857 100%)}"));
    server.sendContent(F("a.btn-logs:hover{background:linear-gradient(135deg,#34d399 0%,#10b981 100%);box-shadow:0 4px 12px rgba(16,185,129,0.25)}"));
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
        streamFileChunked(file, "application/json");
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

void handleGetCurrentGraph() {
    int currentPeriod = (countHours * 60 + countMinutes) / 5;
    JsonDocument doc;
    doc["sh"] = 0;
    doc["sm"] = 0;
    JsonArray array = doc["points"].to<JsonArray>();

    // DEBUG_PRINTF("handleGetCurrentGraph: start=%02d:%02d, currentPeriod=%d\n", startH, startM, currentPeriod);

    for (int period = 0; period <= currentPeriod; period++) {
        int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
        int16_t raw_t1 = 0, raw_t2 = 0, raw_rh = 0;
        eepromReadInt16(currentAddress, raw_t1);
        eepromReadInt16(currentAddress + 2, raw_t2);
        eepromReadInt16(currentAddress + 4, raw_rh);

        if (raw_t1 == 0 && raw_t2 == 0 && raw_rh == 0) continue; 

        JsonObject point = array.add<JsonObject>();
        point["p"] = period;
        point["t1"] = (float)raw_t1 / 10.0;
        point["t2"] = (float)raw_t2 / 10.0;
        point["rh"] = (float)raw_rh;
    }
    
    // DEBUG_PRINTF("Graph points sent: %d\n", array.size());
    
    server.setContentLength(measureJson(doc));
    server.send(200, "application/json", "");
    serializeJson(doc, server.client());
}

void handleArchiveList() {
    server.sendHeader("Connection", "close");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    sendPageHeader("Інкубатор - Архів");

    server.sendContent(F("<div class='container'><h1>Виберіть дату для перегляду</h1>"));
    server.sendContent(F("<a href='/current' class='live' style='margin-bottom:20px;'>Перегляд ПОТОЧНОЇ доби</a>"));
    server.sendContent(F("<ul>"));
    
    std::vector<String> dates;
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        String fileName = dir.fileName();
        if (fileName.startsWith("day_") && fileName.endsWith("_graph.json")) {
            int start = 4;
            int end = fileName.indexOf("_graph.json");
            if (end > start) {
                dates.push_back(fileName.substring(start, end));
            }
        }
    }
    // Сортировка дат хронологически (по месяцу, затем по дню)
    std::sort(dates.begin(), dates.end(), [](const String& a, const String& b) {
        int a_day = a.substring(0, 2).toInt();
        int a_mon = a.substring(3, 5).toInt();
        int b_day = b.substring(0, 2).toInt();
        int b_mon = b.substring(3, 5).toInt();
        if (a_mon != b_mon) return a_mon < b_mon;
        return a_day < b_day;
    });

    for (int i = dates.size() - 1; i >= 0; i--) {
        String dateStr = dates[i];
        String dispDate = dateStr;
        dispDate.replace('_', '.');
        char itemHtml[512];
        snprintf_P(itemHtml, sizeof(itemHtml), 
          PSTR("<li class='archive-item'>"
               "<span style='font-weight:500; font-size:1rem; color:#f8fafc;'>Дата: %s</span>"
               "<span class='btn-group'>"
                 "<a href='/data?day=%s' class='btn-archive btn-graph'>Графік</a>"
                 "<a href='/view_logs?day=%s' class='btn-archive btn-logs'>Логи</a>"
               "</span>"
               "</li>"), 
          dispDate.c_str(), dateStr.c_str(), dateStr.c_str());
        server.sendContent(itemHtml);
        yield();
    }

    server.sendContent(F("</ul><div style='text-align:center;'><a href='/' class='back'>Назад на головну</a></div>"));
    server.sendContent(F("</div></body></html>"));
    server.sendContent("");
}

void formatTimeBuffer(char* buf, size_t size, int period, int sh, int sm) {
  int totalMinutes = (sh * 60 + sm + period * 5) % 1440;
  snprintf_P(buf, size, PSTR("%02d:%02d"), totalMinutes / 60, totalMinutes % 60);
}

void handleShowData() {
    if (!server.hasArg("day") || server.arg("day") == "") {
        server.send(400, "text/plain", F("Bad Request: 'day' parameter is missing"));
        return;
    }
    String day = server.arg("day");


    String statsFilename = "/day_" + day + "_stats.json";
    File statsFile = LittleFS.open(statsFilename, "r");
    JsonDocument statsDoc;
    if (statsFile) {
        deserializeJson(statsDoc, statsFile);
        statsFile.close();
    }

    server.sendHeader("Connection", "close");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    sendPageHeader("Інкубатор - Архів за " + day);

    server.sendContent(F("<div class='container'><h1 style='text-align:center;'>Дані інкубації за "));
    server.sendContent(day); 
    server.sendContent(F("</h1>"));
    server.sendContent(F("<div class='chart-container'><canvas id='tempChart'></canvas></div>"));
    server.sendContent(F("<div style='text-align:center;'><a href='/archive' class='back'>Назад до списку</a></div>"));
    server.sendContent(F("<script>"));
    server.sendContent("const dayNum = \"" + day + "\";");
    server.sendContent("const sh = 0;");
    server.sendContent("const sm = 0;");
    server.sendContent(F(R"raw(
    fetch('/get_graph?day=' + dayNum)
      .then(r => r.json())
      .then(data => {
        const labels = data.map(p => {
            let total = (sh * 60 + sm + p.p * 5) % 1440;
            return Math.floor(total / 60).toString().padStart(2, '0') + ':' + (total % 60).toString().padStart(2, '0');
        });
        const t1 = data.map(p => p.t1);
        const t2 = data.map(p => p.t2);
        const rh = data.map(p => p.rh);
        Chart.defaults.color = '#94a3b8';
        Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.08)';
        new Chart(document.getElementById('tempChart'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [
              { label: 'T1 (°C)', data: t1, borderColor: '#ef4444', backgroundColor: '#ef4444', tension: 0.3, pointRadius: 1, borderWidth: 2 },
              { label: 'T2 (°C)', data: t2, borderColor: '#10b981', backgroundColor: '#10b981', tension: 0.3, pointRadius: 1, borderWidth: 2 },
              { label: 'Вологість (%)', data: rh, borderColor: '#3b82f6', backgroundColor: '#3b82f6', tension: 0.3, pointRadius: 1, borderWidth: 2, yAxisID: 'y1' }
            ]
          },
          options: { 
            responsive: true, 
            maintainAspectRatio: false, 
            scales: { 
              y: { type: 'linear', display: true, position: 'left', title: { display: true, text: 'Темп. (°C)', color: '#94a3b8' }, grid: { color: 'rgba(255, 255, 255, 0.08)' }, ticks: { color: '#94a3b8', callback: function(value){return value.toFixed(1);} } },
              y1: { type: 'linear', display: true, position: 'right', min: 0, max: 100, grid: { drawOnChartArea: false }, title: { display: true, text: 'Волог. (%)', color: '#94a3b8' }, ticks: { color: '#94a3b8' } },
              x: { grid: { color: 'rgba(255, 255, 255, 0.08)' }, ticks: { color: '#94a3b8' } }
            },
            plugins: {
              legend: { labels: { color: '#f8fafc' } }
            }
          } 
        });
      });
    )raw"));
    server.sendContent(F("</script>"));
    server.sendContent(F("<table><tr><th>Час</th><th>T1 (°C)</th><th>T2 (°C)</th><th>Вологість (%)</th></tr>"));

    if (!statsDoc.isNull()) {
        char summary[256];
        snprintf(summary, sizeof(summary), "<tr><th>Статистика</th><td>Avg: %.1f<br>Min: %.1f<br>Max: %.1f</td><td>Avg: %.1f<br>Min: %.1f<br>Max: %.1f</td><td>Avg: %.1f<br>Min: %.1f<br>Max: %.1f</td></tr>",
                 statsDoc["avg_t1"].as<float>(), statsDoc["min_t1"].as<float>(), statsDoc["max_t1"].as<float>(),
                 statsDoc["avg_t2"].as<float>(), statsDoc["min_t2"].as<float>(), statsDoc["max_t2"].as<float>(),
                 statsDoc["avg_rh"].as<float>(), statsDoc["min_rh"].as<float>(), statsDoc["max_rh"].as<float>());
        server.sendContent(summary);
    }
    
    File graphFile = LittleFS.open("/day_" + day + "_graph.json", "r");
    if (graphFile) {
        JsonDocument tempDoc;
        if (!deserializeJson(tempDoc, graphFile)) {
           JsonArray array = tempDoc.as<JsonArray>();
           for (int i = array.size() - 1; i >= 0; i--) {
                JsonObject point = array[i];
                char row[128];
                char fmtTime[32];
                formatTimeBuffer(fmtTime, sizeof(fmtTime), point["p"].as<int>(), 0, 0);
                snprintf(row, sizeof(row), "<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>", 
                         fmtTime, point["t1"].as<float>(), point["t2"].as<float>(), point["rh"].as<float>());
                server.sendContent(row);
                if (i % 20 == 0) yield();
           }
        }
        graphFile.close();
    }
    server.sendContent(F("</table><div style='text-align:center;'><a href='/archive' class='back'>Назад</a></div></div></body></html>"));
    server.sendContent("");
}

void handleCurrentData() {

    int currentPeriod = (countHours * 60 + countMinutes) / 5;
    server.sendHeader("Connection", "close");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    sendPageHeader("Інкубатор - Поточна доба");
    server.sendContent(F("<div class='container'><h1>Дані за поточну добу</h1>"));
    server.sendContent(F("<div class='chart-container'><canvas id='tempChart'></canvas></div>"));
    server.sendContent(F("<div style='text-align:center;'><a href='/archive' class='back'>Назад до списку</a></div>"));
    
    server.sendContent(F("<script>"));
    server.sendContent("const sh = 0;");
    server.sendContent("const sm = 0;");
    server.sendContent(F(R"raw(
    fetch('/get_current_graph')
      .then(r=>r.json())
      .then(json=>{
        const data = json.points;
        const labels = data.map(p => {
            let total = (sh * 60 + sm + p.p * 5) % 1440;
            return Math.floor(total / 60).toString().padStart(2, '0') + ':' + (total % 60).toString().padStart(2, '0');
        });
        Chart.defaults.color = '#94a3b8';
        Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.08)';
        new Chart(document.getElementById('tempChart'), {
          type:'line',
          data:{
            labels:labels,
            datasets:[
              {label:'T1 (°C)', data:data.map(p=>p.t1), borderColor:'#ef4444', backgroundColor:'#ef4444', tension:0.3, pointRadius:1, borderWidth:2},
              {label:'T2 (°C)', data:data.map(p=>p.t2), borderColor:'#10b981', backgroundColor:'#10b981', tension:0.3, pointRadius:1, borderWidth:2},
              {label:'Вологість (%)', data:data.map(p=>p.rh), borderColor:'#3b82f6', backgroundColor:'#3b82f6', tension:0.3, pointRadius:1, borderWidth:2, yAxisID:'y1'}
            ]
          },
          options: { 
            responsive: true, 
            maintainAspectRatio: false, 
            scales: { 
              y: { type: 'linear', display: true, position: 'left', title: { display: true, text: 'Темп. (°C)', color: '#94a3b8' }, grid: { color: 'rgba(255, 255, 255, 0.08)' }, ticks: { color: '#94a3b8', callback: function(value){return value.toFixed(1);} } },
              y1: { type: 'linear', display: true, position: 'right', min: 0, max: 100, grid: { drawOnChartArea: false }, title: { display: true, text: 'Волог. (%)', color: '#94a3b8' }, ticks: { color: '#94a3b8' } },
              x: { grid: { color: 'rgba(255, 255, 255, 0.08)' }, ticks: { color: '#94a3b8' } }
            },
            plugins: {
              legend: { labels: { color: '#f8fafc' } }
            }
          } 
        });
      });
    )raw"));
    server.sendContent(F("</script>"));

    server.sendContent(F("<table><tr><th>Час</th><th>T1</th><th>T2</th><th>RH</th></tr>"));
    for (int period = currentPeriod; period >= 0; period--) {
        int16_t t1, t2, rh;
        eepromReadInt16(DAILY_DATA_START + period * DAILY_DATA_REC_SIZE, t1);
        eepromReadInt16(DAILY_DATA_START + period * DAILY_DATA_REC_SIZE + 2, t2);
        eepromReadInt16(DAILY_DATA_START + period * DAILY_DATA_REC_SIZE + 4, rh);
        if (t1 == 0 && t2 == 0) continue;
        
        char fmtTime[32];
        formatTimeBuffer(fmtTime, sizeof(fmtTime), period, 0, 0);
        char row[128];
        snprintf(row, sizeof(row), "<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>", fmtTime, t1/10.0, t2/10.0, (float)rh);
        server.sendContent(row);
        yield();
    }
    server.sendContent(F("</table><div style='text-align:center;'><a href='/archive' class='back'>Назад</a></div></div></body></html>"));
    server.sendContent("");
}

void handleViewLogs() {
    File file = LittleFS.open("/logs.html", "r");
    if (!file) {
        server.send(404, "text/plain", "logs.html not found");
        return;
    }
    streamFileChunked(file, "text/html");
    file.close();
}

void handleClearLogs() {
    String dateStr = "";
    if (server.hasArg("day") && server.arg("day") != "") {
        dateStr = server.arg("day");
    } else if (server.hasArg("date") && server.arg("date") != "") {
        dateStr = server.arg("date");
    } else {
        if (RTCENABLE) {
            DateTime curr = rtc.now();
            char buf[8];
            snprintf(buf, sizeof(buf), "%02d_%02d", curr.day(), curr.month());
            dateStr = String(buf);
        } else {
            dateStr = "01_01";
        }
    }
    String logFilename = "/day_" + dateStr + "_log.txt";
    LittleFS.remove(logFilename);
    logEvent("Файл логу %s очищено.", logFilename.c_str());
    
    // Перенаправляем обратно в зависимости от того, какой параметр был передан
    server.sendHeader("Location", "/view_logs?day=" + dateStr);
    server.send(303);
}

void handleLogs() {
    String dateStr = "";
    if (server.hasArg("day") && server.arg("day") != "") {
        dateStr = server.arg("day");
    } else {
        if (RTCENABLE) {
            DateTime curr = rtc.now();
            char buf[8];
            snprintf(buf, sizeof(buf), "%02d_%02d", curr.day(), curr.month());
            dateStr = String(buf);
        } else {
            dateStr = "01_01";
        }
    }
    String logFilename = "/day_" + dateStr + "_log.txt";
    if (LittleFS.exists(logFilename)) {
        File file = LittleFS.open(logFilename, "r");
        streamFileChunked(file, "text/plain");
        file.close();
    } else {
        server.send(404, "text/plain", "Log file not found");
    }
}

void streamFileChunked(File& file, const String& contentType) {
    server.sendHeader("Connection", "close");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, contentType, "");
    
    auto client = server.client();
    if (!client) return;

    uint8_t buffer[1024];
    while (file.available()) {
        if (!client.connected()) break; // Прекращаем чтение, если клиент отключился
        size_t len = file.read(buffer, sizeof(buffer));
        if (len > 0) {
            char hexBuf[16];
            snprintf(hexBuf, sizeof(hexBuf), "%X\r\n", (unsigned int)len);
            client.write((const uint8_t*)hexBuf, strlen(hexBuf));
            client.write(buffer, len);
            client.write((const uint8_t*)"\r\n", 2);
        }
        yield();
    }
    if (client.connected()) {
        client.write((const uint8_t*)"0\r\n\r\n", 5);
    }
}

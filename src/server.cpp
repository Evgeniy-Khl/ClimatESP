// server.cpp
#include <main.h>
#include "server.h"

extern uint8_t  seconds, mode, quarter;
extern long lastSendTime;
extern int tableData[32][4], tmrTelegramOff;

void notFoundHandler() {
  server.send(404, "text/plain", "Not found");
}

String getFloat(float val, uint8_t brackets) {
  char buffer[8];
  if(brackets) snprintf(buffer, sizeof(buffer), "[%.1f]", val);
  else snprintf(buffer, sizeof(buffer), "%.1f", val);
  for (unsigned int i = 0; i < sizeof(buffer); i++) {
    if (buffer[i] == '.') {
      buffer[i] = ',';
      break;
    }
  }
  return String(buffer); // Возвращаем отформатированную строку
}

void respondsValues() {
    String string, jsonResponse;
    tmrTelegramOff = 300;
    JsonDocument data;
    data["model"] = "Клімат-5.25&nbsp;&nbsp;&nbsp;&nbsp;№ " + String(settings.sp_structs[1].special & 0x0F);
    data["temperature0"] = getFloat((float)ds[0].pvT/10,0);
    data["settemp0"] = getFloat((float)settings.sp_structs[0].spT/10,1);
    if(detectedSensor == SENSOR_DS18B20 && numberOfDevices > 1){
        data["temperature1"] = getFloat((float)ds[1].pvT/10,0);
        data["settemp1"] = getFloat((float)settings.sp_structs[1].spT/10,1);
        if(pvRH == 255) {data["humidity"] = "не визначено"; data["sethum"] = " ";}
        else {data["humidity"] = String(pvRH);data["sethum"] = "%";}
        if(HIH5030) data["sethum"] = getFloat((float)settings.sp_structs[1].spRH/10,1);
    } else if(detectedSensor == SENSOR_DHT22){
      data["humidity"] = getFloat((float)ds[1].pvT/10,0);
      data["sethum"] = getFloat((float)settings.sp_structs[1].spRH/10,1);
    }
    
    switch (settings.sp_structs[1].extendMode){
      case 1: string = "охолодження"; break;
      case 2: string = "осущення"; break;
      case 3: string = "охол. и осуш."; break;
      default: string = ""; break;
      }
    data["ventmode"] = string;

    switch (settings.sp_structs[0].extendMode){
      case 0: string = "сирена"; break;
      case 1: string = "відключення"; break;
      default: string = ""; break;
    }
    data["extmode"] = string;
    
    switch (settings.sp_structs[1].mode){
      case 0: string = "немає"; break;
      case 1: string = "канал №1"; break;
      case 2: string = "канал №2"; break;
      case 3: string = "№1 и №2"; break;
      case 4: string = "імпульс"; break;
      default: string = ""; break;
    }
    data["relaymode"] = string;
    data["checkDry"] = (settings.sp_structs[0].mode) ? "встановлене" : "немає";
    data["rotation"] = String(pvTimer) + (TURNSECOND ? " сек." : " хвл.");

    data["power"] = String(pctHeater) + "%";
    data["error1"] = ERROR1;
    data["error2"] = ERROR2;
    data["error4"] = ERROR4;
    data["error8"] = ERROR8;
    data["overheat"] = OVERHEAT;
    data["flap"] = String(pvFlap) + "%";
    if(settings.sp_structs[1].state==0) string = "немає";
    else string = "№"+String(settings.sp_structs[1].state);
    data["program"] = string;
    data["currDay"] = "0 діб.";//String(upv.pv.currDay) + "діб.";
    data["led0"] = dataLed[0] ? "OFF" : "ON" ;  // Поворот лотков
    data["led1"] = dataLed[1] ? "OFF" : "ON" ;  // НАГРЕВАТЕЛЬ
    data["led2"] = dataLed[2] ? "OFF" : "ON" ;  // УВЛАЖНИТЕЛЬ
    data["led3"] = dataLed[3] ? "OFF" : "ON" ;  // Заслонка/вентилятор охлаждения
    data["led4"] = dataLed[4] ? "OFF" : "ON" ;  // Вспомогательный нагреватель
    data["led5"] = dataLed[5] ? "OFF" : "ON" ;  // Авария
    
    serializeJson(data, jsonResponse);
    // DEBUG_PRINTF("SERVER responds to the client with VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
    // MYDEBUG_PRINTLN("out=" + response);
    server.send(200, "application/json", jsonResponse);
    // DEBUG_PRINTF("END VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
}

void respondsEeprom(){
    String jsonResponse;
    JsonDocument doc;
        doc["spT0"] = settings.sp_structs[0].spT;
        doc["spT1"] = settings.sp_structs[1].spT;
        doc["spRH0"] = settings.sp_structs[0].spRH;
        doc["spRH1"] = settings.sp_structs[1].spRH;
        doc["ventMode"] = settings.sp_structs[1].extendMode;
        doc["extendMode"] = settings.sp_structs[0].extendMode;
        doc["relayMode"] = settings.sp_structs[1].mode;
        doc["checkDry"] = settings.sp_structs[0].mode;
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

        serializeJson(doc, jsonResponse); // Сериализуем JSON
        DEBUG_PRINTF("SERVER responds to the client with EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
        MYDEBUG_PRINTLN(jsonResponse);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
        server.send(200, "application/json", jsonResponse); // Отправляем ответ
        // DEBUG_PRINTF("END EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
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
      else if (paramName == "relayMode") settings.sp_structs[1].mode = paramValue.toInt();
      else if (paramName == "checkDry") settings.sp_structs[0].mode = paramValue.toInt();
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
    String jsonResponse;
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
      serializeJson(doc, jsonResponse);
      DEBUG_PRINTF("SERVER responds to the client PROGRAM DATA #: %d,%ld\n",seconds,millis()-lastSendTime);
      MYDEBUG_PRINTLN("jsonResponse:"+jsonResponse);
      server.send(200, "application/json", jsonResponse);
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
 * @brief Генерирует HTML-страницу со списком всех архивных файлов (_graph.json).
 */
void handleArchiveList() {
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Инкубатор - Архив</title><style>body{font-family:Arial,sans-serif;background-color:#f4f4f4}div{max-width:600px;margin:40px auto;padding:20px;background:#fff;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}h1{text-align:center;color:#333}ul{list-style-type:none;padding:0}li{margin:10px 0}a{display:block;padding:15px;background:#007bff;color:white;text-align:center;text-decoration:none;border-radius:5px;transition:background-color .3s}a:hover{background-color:#0056b3}a.back{background-color:#6c757d}a.back:hover{background-color:#5a6268}</style></head><body><div><h1>Выберите день для просмотра</h1><ul>";
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        String fileName = dir.fileName();
        if (fileName.startsWith("/day_") && fileName.endsWith("_graph.json")) {
            int start = 5;
            int end = fileName.indexOf('_', start);
            String day = fileName.substring(start, end);
            html += "<li><a href='/data?day=" + day + "'>Просмотр данных за День " + day + "</a></li>";
        }
    }
    html += "</ul><a href='/' class='back' style='margin-top: 20px;'>Назад на головну</a></div></body></html>";
    server.send(200, "text/html", html);
}


/**
 * @brief Генерирует HTML-страницу с таблицей данных для конкретного дня.
 * День передается как GET-параметр, например: /data?day=21
 */
void handleShowData() {
    if (!server.hasArg("day")) {
        server.send(400, "text/plain", "Bad Request: Missing 'day' parameter");
        return;
    }
    String day = server.arg("day");
    String filename = "/day_" + day + "_graph.json";

    File file = LittleFS.open(filename, "r");
    if (!file) {
        server.send(404, "text/plain", "File Not Found: " + filename);
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        server.send(500, "text/plain", "JSON Parse Error");
        return;
    }

    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Инкубатор - День " + day + "</title><style>body{font-family:Arial,sans-serif}table{border-collapse:collapse;width:50%;margin:20px auto}th,td{border:1px solid #ddd;text-align:center;padding:8px}th{background-color:#f2f2f2}tr:nth-child(even){background-color:#f9f9f9}a{display:block;text-align:center;margin-bottom:20px;font-size:18px}</style></head><body><h1 style='text-align:center;'>Данные инкубации за День " + day + "</h1><a href='/archive'>Назад к списку дней</a><table><tr><th>Период (5 мин)</th><th>Температура T1 (°C)</th><th>Температура T2 (°C)</th></tr>";
    JsonArray array = doc.as<JsonArray>();
    for (JsonObject point : array) {
        html += "<tr><td>" + String(point["p"].as<int>()) + "</td><td>" + String(point["t1"].as<float>(), 1) + "</td><td>" + String(point["t2"].as<float>(), 1) + "</td></tr>";
    }
    html += "</table></body></html>";
    server.send(200, "text/html", html);
}
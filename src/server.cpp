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
    data["model"] = "Клімат-5.25&nbsp;&nbsp;&nbsp;&nbsp;№:" + String(settings.sp_structs[1].special);
    data["temperature0"] = getFloat((float)ds[0].pvT/10,0);
    data["temperature1"] = getFloat((float)ds[1].pvT/10,0);
    data["settemp0"] = getFloat((float)settings.sp_structs[0].spT/10,1);
    data["settemp1"] = getFloat((float)settings.sp_structs[1].spT/10,1);
    if(pvRH == 255) data["humidity"] = "***";
    else data["humidity"] = String(pvRH);
    if(HIH5030 || AM2301) data["sethum"] = "[" + String(settings.sp_structs[1].spRH) + "]";
    else  data["sethum"] = "[--]";
    
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
    data["flap"] = String(pvFlap) + "%";
    if(settings.sp_structs[1].state==0) string = "немає";
    else string = "№"+String(settings.sp_structs[1].state);
    data["program"] = string;
    data["currDay"] = "0 діб.";//String(upv.pv.currDay) + "діб.";
    data["led0"] = dataLed[0] ? "ON" : "OFF" ;  // НАГРЕВАТЕЛЬ
    data["led1"] = dataLed[1] ? "ON" : "OFF" ;  // УВЛАЖНИТЕЛЬ
    data["led2"] = dataLed[2] ? "ON" : "OFF" ;  // Поворот лотков
    data["led3"] = dataLed[3] ? "ON" : "OFF" ;  // Заслонка/вентилятор охлаждения
    data["led4"] = dataLed[4] ? "ON" : "OFF" ;  // Вспомогательный нагреватель
    data["led5"] = dataLed[5] ? "ON" : "OFF" ;  // Авария
    
    serializeJson(data, jsonResponse);
    // DEBUG_PRINTF("SERVER responds to the client with VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
    // Serial.println("out=" + response);
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
        DEBUG_PRINTLN(jsonResponse);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
        server.send(200, "application/json", jsonResponse); // Отправляем ответ
        // DEBUG_PRINTF("END EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
}

void acceptEeprom() {
  // Логирование всех параметров
  DEBUG_PRINTF("The SERVER has accepted EEPROM: %d, %ld\n", seconds, millis() - lastSendTime);
  
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
      else if (paramName == "coolOn0") settings.sp_structs[0].coolOn = paramValue.toInt();    // "vent0"
      else if (paramName == "coolOff0") settings.sp_structs[0].coolOff = paramValue.toInt();  // "vent1"
      else if (paramName == "coolOn1") settings.sp_structs[1].coolOn = paramValue.toInt();    // ?????
      else if (paramName == "coolOff1") settings.sp_structs[1].coolOff = paramValue.toInt();  // ?????
      else if (paramName == "extOn") settings.sp_structs[0].auxiliary = paramValue.toInt();
      else if (paramName == "extOff") settings.sp_structs[1].auxiliary = paramValue.toInt();
      else if (paramName == "minRun") settings.sp_structs[0].pulse = paramValue.toInt();
      // else if (paramName == "maxRun") usp.sp.maxRun = paramValue.toInt();
      else if (paramName == "period") settings.sp_structs[1].pulse = paramValue.toInt();
      // else if (paramName == "hysteresis") usp.sp.hysteresis = paramValue.toInt();
      else if (paramName == "air0") settings.sp_structs[0].aeration = paramValue.toInt();
      else if (paramName == "air1") settings.sp_structs[1].aeration = paramValue.toInt();
      else if (paramName == "flpClose") settings.sp_structs[0].flapLimit = paramValue.toInt();
      else if (paramName == "flpOpen") settings.sp_structs[1].flapLimit = paramValue.toInt();
      else if (paramName == "flpNow") settings.sp_structs[0].state = paramValue.toInt();
      else if (paramName == "pkoff0") settings.sp_structs[0].Kp = paramValue.toInt();
      else if (paramName == "pkoff1") settings.sp_structs[1].Kp = paramValue.toInt();
      else if (paramName == "ikoff0") settings.sp_structs[0].Ki = paramValue.toInt();
      else if (paramName == "ikoff1") settings.sp_structs[1].Ki = paramValue.toInt();
      // else if (paramName == "identif") usp.sp.identif = paramValue.toInt();
  }

  server.send(200); // Отправляем только статус 200

  // saveEeprom();
}

  void respondsProgram(){
    String jsonResponse;
    JsonDocument doc;
    mode = SAVEPROG; interval = INTERVAL_1000; quarter = SET_PROG4+1;

    for (int i = 0; i < 32; i++) {
        JsonArray row = doc.add<JsonArray>();
        for (int j = 0; j < 4; j++) {
            row.add(tableData[i][j]);
        }
    }
    serializeJson(doc, jsonResponse);
    DEBUG_PRINTF("SERVER responds to the client with PROGRAM: %d,%ld\n",seconds,millis()-lastSendTime);
    DEBUG_PRINTLN("jsonResponse:"+jsonResponse);
    server.send(200, "application/json", jsonResponse);
  }

  //https://arduinojson.org/v7/assistant/#/step1
  void programDeser(String input){
    // Stream& input;
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      DEBUG_PRINT("deserializeJson() failed: ");
      DEBUG_PRINTLN(error.c_str());
      return;
    }

    JsonArray data = doc["data"];
    const int rows = 32; // Количество строк
    const int cols = 4;  // Количество столбцов
    DEBUG_PRINTLN("programDeser()");

    for (int i = 0; i < rows; i++) {
      JsonArray data_i = data[i];
      for (int j = 0; j < cols; j++) {
        tableData[i][j] = data_i[j]; // Заполнение массива
        DEBUG_PRINT(tableData[i][j]); DEBUG_PRINT("; ");
      }
      DEBUG_PRINTLN("||");
    }
  }

  void acceptProgram() {
    String jsonData;

    // Проверка наличия параметра "data" в запросе
    if (server.hasArg("data")) {
        jsonData = server.arg("data");
        DEBUG_PRINTLN("jsonData: " + jsonData); // Логирование полученных данных
        
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
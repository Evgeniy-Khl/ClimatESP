// server.cpp
#include <ArduinoJson.h>
#include "server.h"
#include "usart.h"

extern uint8_t dataLed[5], seconds, mode, quarter;
extern long lastSendTime;
extern int tableData[32][4], tmrTelegramOff;
extern Setting usp;

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
    data["model"] = "Клімат-5." + String(upv.pv.model) + "&nbsp;&nbsp;&nbsp;ID:" + String(upv.pv.node);
    data["temperature0"] = getFloat((float)upv.pv.t[0]/10,0);
    data["temperature1"] = getFloat((float)upv.pv.t[1]/10,0);
    data["settemp0"] = getFloat((float)upv.pv.spT[0]/10,1);
    data["settemp1"] = getFloat((float)upv.pv.spT[1]/10,1);
    data["humidity"] = String(upv.pv.rH);
    data["sethum"] = "[" + String(upv.pv.spRH[1]) + "]";
    switch (upv.pv.ventMode){
      case 1: string = "охолодження"; break;
      case 2: string = "осущення"; break;
      case 3: string = "охол. и осуш."; break;
      default: string = ""; break;
      }
    data["ventmode"] = string;
    switch (upv.pv.extMode){
    case 0: string = "сирена"; break;
    case 1: string = "відключення"; break;
    case 2: string = "нагрівач"; break;
    default: string = ""; break;
    }
    data["extmode"] = string;
    switch (upv.pv.relayMode){
      case 0: string = "немає"; break;
      case 1: string = "канал №1"; break;
      case 2: string = "канал №2"; break;
      case 3: string = "№1 и №2"; break;
      case 4: string = "імпульс"; break;
      default: string = ""; break;
      }
    data["relaymode"] = string;
    data["checkDry"] = (upv.pv.checkDry) ? "встановлене" : "немає";
    data["rotation"] = String(upv.pv.timer) + "хвл.";
    data["power"] = String(upv.pv.power) + "%";
    data["flap"] = String(upv.pv.flap) + "%";
    if(upv.pv.program==0) string = "немає";
    else string = "№"+String(upv.pv.program);
    data["program"] = string;
    data["currDay"] = String(upv.pv.currDay) + "діб.";
    data["led0"] = dataLed[0] ? "ON" : "OFF" ;
    data["led1"] = dataLed[1] ? "ON" : "OFF" ;
    data["led2"] = dataLed[2] ? "ON" : "OFF" ;
    data["led3"] = dataLed[3] ? "ON" : "OFF" ;
    data["led4"] = dataLed[4] ? "ON" : "OFF" ;
    
    serializeJson(data, jsonResponse);
    // Serial.printf("SERVER responds to the client with VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
    // Serial.println("out=" + response);
    server.send(200, "application/json", jsonResponse);
    // Serial.printf("END VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
}

void respondsEeprom(){
    String jsonResponse;
    JsonDocument doc;
        doc["spT0"] = usp.sp.spT[0];
        doc["spT1"] = usp.sp.spT[1];
        doc["spRH0"] = usp.sp.spRH[0];
        doc["spRH1"] = usp.sp.spRH[1];
        doc["ventMode"] = usp.sp.ventMode;
        doc["extendMode"] = usp.sp.extMode;
        doc["relayMode"] = usp.sp.relayMode;
        doc["checkDry"] = usp.sp.checkDry;
        doc["rotate0"] = usp.sp.rotate[0];
        doc["rotate1"] = usp.sp.rotate[1];
        doc["program"] = usp.sp.program;
        doc["alarm0"] = usp.sp.alarm[0];
        doc["alarm1"] = usp.sp.alarm[1];
        doc["vent0"] = usp.sp.vent[0];
        doc["vent1"] = usp.sp.vent[1];
        doc["extOn"] = usp.sp.extOn;
        doc["extOff"] = usp.sp.extOff;
        doc["minRun"] = usp.sp.minRun;
        doc["maxRun"] = usp.sp.maxRun;
        doc["period"] = usp.sp.period;
        doc["hysteresis"] = usp.sp.hysteresis;
        doc["air0"] = usp.sp.air[0];
        doc["air1"] = usp.sp.air[1];
        doc["flpClose"] = usp.sp.flpClose;
        doc["flpOpen"] = usp.sp.flpOpen;
        doc["flpNow"] = usp.sp.flpNow;
        doc["pkoff0"] = usp.sp.pkoff[0];
        doc["pkoff1"] = usp.sp.pkoff[1];
        doc["ikoff0"] = usp.sp.ikoff[0];
        doc["ikoff1"] = usp.sp.ikoff[1];
        doc["identif"] = usp.sp.identif;
        doc["status"] = usp.sp.status;

        serializeJson(doc, jsonResponse); // Сериализуем JSON
        Serial.printf("SERVER responds to the client with EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
        Serial.println(jsonResponse);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
        server.send(200, "application/json", jsonResponse); // Отправляем ответ
        // Serial.printf("END EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
}

void acceptEeprom() {
  // Логирование всех параметров
  Serial.printf("The SERVER has accepted EEPROM: %d, %ld\n", seconds, millis() - lastSendTime);
  
  for (uint8_t i = 0; i < server.args(); i++) {
      String paramName = server.argName(i);
      String paramValue = server.arg(i);
      
      // Логирование параметров (раскомментируйте, если нужно)
      // Serial.printf("Parameter: %s, Value: %s\n", paramName.c_str(), paramValue.c_str());
      
      if (paramName == "spT0") usp.sp.spT[0] = paramValue.toInt();
      else if (paramName == "spT1") usp.sp.spT[1] = paramValue.toInt();
      else if (paramName == "spRH0") usp.sp.spRH[0] = paramValue.toInt();
      else if (paramName == "spRH1") usp.sp.spRH[1] = paramValue.toInt();
      else if (paramName == "ventMode") usp.sp.ventMode = paramValue.toInt();
      else if (paramName == "extendMode") usp.sp.extMode = paramValue.toInt();
      else if (paramName == "relayMode") usp.sp.relayMode = paramValue.toInt();
      else if (paramName == "checkDry") usp.sp.checkDry = paramValue.toInt();
      else if (paramName == "rotate0") usp.sp.rotate[0] = paramValue.toInt();
      else if (paramName == "rotate1") usp.sp.rotate[1] = paramValue.toInt();
      else if (paramName == "program") usp.sp.program = paramValue.toInt();
      else if (paramName == "alarm0") usp.sp.alarm[0] = paramValue.toInt();
      else if (paramName == "alarm1") usp.sp.alarm[1] = paramValue.toInt();
      else if (paramName == "vent0") usp.sp.vent[0] = paramValue.toInt();
      else if (paramName == "vent1") usp.sp.vent[1] = paramValue.toInt();
      else if (paramName == "extOn") usp.sp.extOn = paramValue.toInt();
      else if (paramName == "extOff") usp.sp.extOff = paramValue.toInt();
      else if (paramName == "minRun") usp.sp.minRun = paramValue.toInt();
      else if (paramName == "maxRun") usp.sp.maxRun = paramValue.toInt();
      else if (paramName == "period") usp.sp.period = paramValue.toInt();
      else if (paramName == "hysteresis") usp.sp.hysteresis = paramValue.toInt();
      else if (paramName == "air0") usp.sp.air[0] = paramValue.toInt();
      else if (paramName == "air1") usp.sp.air[1] = paramValue.toInt();
      else if (paramName == "flpClose") usp.sp.flpClose = paramValue.toInt();
      else if (paramName == "flpOpen") usp.sp.flpOpen = paramValue.toInt();
      else if (paramName == "flpNow") usp.sp.flpNow = paramValue.toInt();
      else if (paramName == "pkoff0") usp.sp.pkoff[0] = paramValue.toInt();
      else if (paramName == "pkoff1") usp.sp.pkoff[1] = paramValue.toInt();
      else if (paramName == "ikoff0") usp.sp.ikoff[0] = paramValue.toInt();
      else if (paramName == "ikoff1") usp.sp.ikoff[1] = paramValue.toInt();
      else if (paramName == "identif") usp.sp.identif = paramValue.toInt();
  }

  server.send(200); // Отправляем только статус 200

  saveEeprom();
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
    Serial.printf("SERVER responds to the client with PROGRAM: %d,%ld\n",seconds,millis()-lastSendTime);
    Serial.println("jsonResponse:"+jsonResponse);
    server.send(200, "application/json", jsonResponse);
  }

  //https://arduinojson.org/v7/assistant/#/step1
  void programDeser(String input){
    // Stream& input;
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonArray data = doc["data"];
    const int rows = 32; // Количество строк
    const int cols = 4;  // Количество столбцов
    Serial.println("programDeser()");

    for (int i = 0; i < rows; i++) {
      JsonArray data_i = data[i];
      for (int j = 0; j < cols; j++) {
        tableData[i][j] = data_i[j]; // Заполнение массива
        Serial.print(tableData[i][j]); Serial.print("; ");
      }
      Serial.println("||");
    }
  }

  void acceptProgram() {
    String jsonData;

    // Проверка наличия параметра "data" в запросе
    if (server.hasArg("data")) {
        jsonData = server.arg("data");
        Serial.println("jsonData: " + jsonData); // Логирование полученных данных
        
        // Отправляем статус 200
        server.send(200); 
        
        // Обработка полученных данных
        programDeser(jsonData);
        mode = SAVEPROG; interval = INTERVAL_1000;
        quarter = SET_PROG1;
        
        Serial.printf("Accept Program: %d, %ld\n", seconds, millis() - lastSendTime);
    } else {
        // Отправка сообщения об ошибке, если параметр отсутствует
        server.send(400, "text/plain", "Ошибка: нет данных");
    }
  }
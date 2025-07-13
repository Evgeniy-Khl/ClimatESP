#include "main.h"
void initWiFiManag(void){
    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_botToken("botToken", "BOT token", botToken, 50);
    WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID, 11);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //add all your parameters here
    wifiManager.addParameter(&custom_botToken);
    wifiManager.addParameter(&custom_chatID);

    //------------------ reset settings ------------------------
    if(settings.sp_structs[1].extendMode & 0x10){
      settings.sp_structs[1].extendMode &= 0xEF;
      saveConfig();
      wifiManager.resetSettings();
    } 
    //----------------------------------------------------------
    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();
    //----------------------------------------------------------
    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    wifiManager.setTimeout(20);
    //----------------------------------------------------------
    //получает SSID и пароль и пытается подключиться
    //если подключение не удаётся, запускает точку доступа с указанным именем "AutoConnectAP"
    //и переходит в блокирующий цикл ожидания настройки
    if (!wifiManager.autoConnect("AutoConnectAP")) {
      DEBUG_PRINTLN("failed to connect and hit timeout");
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
    }

    //------- if you get here you have connected to the WiFi -----------
    DEBUG_PRINT("connected...yeey   local ip:");
    DEBUG_PRINTLN(WiFi.localIP());	// Print ESP32 Local IP Address
    IPAddress myIP = WiFi.localIP();
    for (int i = 0; i < 4; i++) {
      dataLed[i] = myIP[i];
      DEBUG_PRINT("ip:"); DEBUG_PRINTLN(dataLed[i]);
    }
    //------------------ read updated parameters -----------------------
    strcpy(botToken, custom_botToken.getValue());
    strcpy(chatID, custom_chatID.getValue());
    DEBUG_PRINTLN("----The values in the file are ----");
    DEBUG_PRINTLN("botToken:" + String(botToken));
    DEBUG_PRINTLN("chatID:" + String(chatID));
    DEBUG_PRINTLN();
    //-------------- Проверяем, что botToken не пустая -----------------
    if (strlen(botToken) > 0) {
        bot.updateToken(botToken);
        // if(botSetup()) Serial.println("The command list was updated successfully.");
        bot.sendMessage(chatID, "Climate-5.25", "");//bot.sendMessage("25235518", "Hola amigo!", "Markdown");
    }
    //--------------- save the custom parameters to FS -------------------------
    if(shouldSaveConfig) {
      DEBUG_PRINTLN("saving config");
      JsonDocument json;
      json["botToken"] = botToken;
      json["chatID"] = chatID;
      File configFile = LittleFS.open("/config.json", "w");
      if (!configFile) {
        DEBUG_PRINTLN("failed to open config file for writing");
      }
      serializeJson(json, Serial);
      serializeJson(json, configFile);
      configFile.close();
      
    }
  //============================== END SAVE =====================================
    server.on("/", HTTP_GET, []() {
      mode = READDEFAULT; interval = INTERVAL_4000; tmrTelegramOff = 300;
      if (!LittleFS.exists("/index.html")) {
        DEBUG_PRINTLN("index.html not found");
      } else {
        File file = LittleFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "I can't open the index.html");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
      }
    });
    server.on("/setup", HTTP_GET, []() {
      DEBUG_PRINTF("/setup ----- EEPROM size: %d;  time: %d,%ld\n", EEPROM_SIZE,seconds,millis()-lastSendTime);
      File file = LittleFS.open("/setup.html", "r");
      if (!file) {
          server.send(404, "text/plain", "File Not Found");
          return;
      }
      server.streamFile(file, "text/html");
      file.close();
    });
    server.on("/table", HTTP_GET, []() {
      DEBUG_PRINTF("/setup ----- EEPROM size: %d;  time: %d,%ld\n", EEPROM_SIZE,seconds,millis()-lastSendTime);
      File file = LittleFS.open("/table.html", "r");
      if (!file) {
          server.send(404, "text/plain", "File Not Found");
          return;
      }
      server.streamFile(file, "text/html");
      file.close();
    });
    server.on("/getvalues", HTTP_GET, respondsValues);      // the server responds the completed index.html to the client
    server.on("/geteeprom", HTTP_GET, respondsEeprom);      // the server responds the completed setup.html to the client
    server.on("/seteeprom", HTTP_POST, acceptEeprom);       // the server accepts the edited setup.html from the client
    server.on("/get_table", HTTP_POST, respondsProgram);    // the server responds the completed table.html to the client
    server.on("/save_table", HTTP_GET, acceptProgram);      // the server accepts the edited table.html from the client
    server.onNotFound(notFoundHandler);
    
    server.begin();   // Start server
    DEBUG_PRINTLN("HTTP server started");
    
    begHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
    DEBUG_PRINTF("Free heap size: %d\n", begHeapSize);
}
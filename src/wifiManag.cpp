#include "main.h"
#include "server.h"
void initWiFiManag(void){
    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_botToken("botToken", "BOT token", botToken, 50);
    WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID, 11);

    //WiFiManager https://github.com/tzapu/WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //add all your parameters here
    wifiManager.addParameter(&custom_botToken);
    wifiManager.addParameter(&custom_chatID);

    //------------------ reset settings ------------------------
    if(settings.sp_structs[0].special & 0x08){
      settings.sp_structs[0].special &= 0xF7;
      saveSetpoint();
      MYDEBUG_PRINTLN("Сброс настроек WiFi!");
      wifiManager.resetSettings();
    } 
    //----------------------------------------------------------
    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();
    //----------------------------------------------------------
    uint8_t tt = (settings.sp_structs[0].special & 0x03) * 60;
    MYDEBUG_PRINT("Устанавливаем таймаут для портала конфигурации: "); MYDEBUG_PRINTLN(tt);
    data[0] = 0b00111001; // (67)	  C
    data[1] = 0b01011100; // (111)	o
    data[2] = 0b01010100; // (110)	n
    if(tt/100) data[3] = NUMBER_FONT[tt/100];
    data[4] = NUMBER_FONT[(tt/10)%10];
    data[5] = NUMBER_FONT[tt%10];
    data[6] = 0b00111110; // (85)	U
    data[7] = 0b01110001; // (70)	F
    module.setDisplay(data, 8);
    wifiManager.setConfigPortalTimeout(tt);    
    //----------------------------------------------------------
    // Пытаемся подключиться
    if (!wifiManager.autoConnect("ClimatAP")) {
      MYDEBUG_PRINTLN("Не удалось подключиться (истек таймаут). Продолжаем работу в оффлайн-режиме.");
      data[3] = DEF;
      data[4] = DEF;
      data[5] = DEF;  // ---
      module.setDisplay(data, 8);
      delay(1000);
      // Ничего не делаем здесь, чтобы программа просто продолжила выполнение
    } else {
        //------- if you get here you have connected to the WiFi -----------
        WIFIENABLE = 1;
        MYDEBUG_PRINT("Wi-Fi успешно подключен! Local ip:");
        MYDEBUG_PRINTLN(WiFi.localIP());	// Print ESP32 Local IP Address
        data[3] = GR;
        data[4] = GR;
        data[5] = GR;  // ooo
        module.setDisplay(data, 8);
        delay(1000);
        client.setInsecure();        // Говорим клиенту не проверять сертификат
        //------------------ read updated parameters -----------------------
        strcpy(botToken, custom_botToken.getValue());
        strcpy(chatID, custom_chatID.getValue());
        MYDEBUG_PRINTLN("----The values in the file are ----");
        MYDEBUG_PRINTLN("botToken:" + String(botToken));
        MYDEBUG_PRINTLN("chatID:" + String(chatID));
        MYDEBUG_PRINTLN();
        //-------------- Проверяем, что botToken не пустая -----------------
        if (strlen(botToken) > 40) {
            bot.updateToken(botToken);
            MYDEBUG_PRINTLN("bot.updateToken:" + bot.getToken());
            // if(botSetup()) MYDEBUG_PRINTLN("The command list was updated successfully.");
            uint16_t begHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
            DEBUG_PRINTF("Free heap size: %d\n", begHeapSize);
            String statusMess = WORD_TITLE + String(settings.sp_structs[1].special) + NEW_STR;
            statusMess += WORD_IP + WiFi.localIP().toString() + NEW_STR;
            statusMess += GRAVE_ACCENT;
            bot.sendMessage(chatID, statusMess, "Markdown");
            BOTENABLE = 1;
            MYDEBUG_PRINTLN("bot.updateToken!");
        }
        //--------------- save the custom parameters to FS -------------------------
        if(shouldSaveConfig) {
          MYDEBUG_PRINTLN("saving config");
          JsonDocument json;
          json["botToken"] = botToken;
          json["chatID"] = chatID;
          File configFile = LittleFS.open("/config.json", "w");
          if (!configFile) {
            MYDEBUG_PRINTLN("failed to open config file for writing");
          }
          serializeJson(json, Serial);
          serializeJson(json, configFile);
          configFile.close();
        }
        //============================== END =====================================
        server.on("/", HTTP_GET, []() {
          mode = READDEFAULT; interval = INTERVAL_4000; tmrTelegramOff = 300;
          if (!LittleFS.exists("/index.html")) {
            MYDEBUG_PRINTLN("index.html not found");
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
        server.on("/archive", HTTP_GET, handleArchiveList); // Генерирует HTML-страницу со списком всех архивных файлов (_graph.json).
        server.on("/data", HTTP_GET, handleShowData);       // Генерирует страницу с таблицей, отправляя ВЕСЬ HTML по частям.
        server.on("/get_graph", HTTP_GET, handleGetGraph);   // Отдает сырой JSON файл для графика.
        server.on("/get_current_graph", HTTP_GET, handleGetCurrentGraph); 
        server.on("/current", HTTP_GET, handleCurrentData); // Генерирует таблицу с данными за ТЕКУЩИЙ день, читая их напрямую из AT24C32.
        server.on("/getvalues", HTTP_GET, respondsValues);      // the server responds the completed index.html to the client
        server.on("/geteeprom", HTTP_GET, respondsEeprom);      // the server responds the completed setup.html to the client
        server.on("/seteeprom", HTTP_POST, acceptEeprom);       // the server accepts the edited setup.html from the client
        server.on("/get_table", HTTP_POST, respondsProgram);    // the server responds the completed table.html to the client
        server.on("/save_table", HTTP_GET, acceptProgram);      // the server accepts the edited table.html from the client
        server.onNotFound(notFoundHandler);
        
        server.begin();   // Start server
        MYDEBUG_PRINTLN("HTTP server started");
        
        uint16_t begHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
        DEBUG_PRINTF("Free heap size: %d\n", begHeapSize);
    }
}
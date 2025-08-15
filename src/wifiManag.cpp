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
      saveSetpoint();
      wifiManager.resetSettings();
    } 
    //----------------------------------------------------------
    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();
    //----------------------------------------------------------
    if(settings.sp_structs[0].special < 60) settings.sp_structs[0].special = 60;
    
    MYDEBUG_PRINT("Устанавливаем таймаут для портала конфигурации: "); MYDEBUG_PRINTLN(settings.sp_structs[0].special);
    // Устанавливаем таймаут для портала конфигурации в 60 секунд (1 минута)
    wifiManager.setConfigPortalTimeout(settings.sp_structs[0].special);    
    //----------------------------------------------------------
    // Пытаемся подключиться
    if (!wifiManager.autoConnect("AutoConnectAP")) {
      MYDEBUG_PRINTLN("Не удалось подключиться (истек таймаут). Продолжаем работу в оффлайн-режиме.");
      // Ничего не делаем здесь, чтобы программа просто продолжила выполнение
    } else {
        //------- if you get here you have connected to the WiFi -----------
        MYDEBUG_PRINT("Wi-Fi успешно подключен! Local ip:");
        MYDEBUG_PRINTLN(WiFi.localIP());	// Print ESP32 Local IP Address
        IPAddress myIP = WiFi.localIP();
        for (int i = 0; i < 4; i++) {
          dataLed[i] = myIP[i];
          // MYDEBUG_PRINT("ip:"); MYDEBUG_PRINTLN(dataLed[i]);
        }
       #ifdef ESP8266
          X509List cert(TELEGRAM_CERTIFICATE_ROOT);
          client.setTrustAnchors(&cert);      // Add root certificate for api.telegram.org
       #endif
        //------------------- Настройки времени ----------------------------
        const char* ntpServer = "pool.ntp.org"; // Сервер NTP
        // Строка конфигурации часового пояса для Украины (EET/EEST)
        // EET-2EEST,M3.5.0/3,M10.5.0/4
        // EET-2: Стандартное время UTC+2
        // EEST: Летнее время
        // M3.5.0/3: Переход на летнее время в 3:00 в последнее воскресенье марта
        // M10.5.0/4: Переход на зимнее время в 4:00 в последнее воскресенье октября
        const char* tzInfo = "EET-2EEST,M3.5.0/3,M10.5.0/4";
        // Конфигурируем и запускаем синхронизацию времени
        // configTime(0, 0, "pool.ntp.org");   // get UTC time via NTP
        // configTime(смещение_в_секундах, смещение_для_летнего_времени, ntp_сервер) - устаревший метод
        // Новый, правильный метод использует строку часового пояса:
        configTime(tzInfo, ntpServer);
        // Ожидаем, пока время будет получено
        MYDEBUG_PRINT("Waiting for time synchronization");
        while (!time(nullptr)) {
          MYDEBUG_PRINT(".");
          delay(1000);
        }
        MYDEBUG_PRINTLN("\nTime synchronized!");

        //------------------ read updated parameters -----------------------
        strcpy(botToken, custom_botToken.getValue());
        strcpy(chatID, custom_chatID.getValue());
        MYDEBUG_PRINTLN("----The values in the file are ----");
        MYDEBUG_PRINTLN("botToken:" + String(botToken));
        MYDEBUG_PRINTLN("chatID:" + String(chatID));
        MYDEBUG_PRINTLN();
        //-------------- Проверяем, что botToken не пустая -----------------
        if (strlen(botToken) > 0) {
            bot.updateToken(botToken);
            // if(botSetup()) Serial.println("The command list was updated successfully.");
            bot.sendMessage(chatID, "Climate-5.25", "");//bot.sendMessage("25235518", "Hola amigo!", "Markdown");
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
        //============================== END SAVE =====================================
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
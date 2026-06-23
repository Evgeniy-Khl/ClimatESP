#include "main.h"
#include "server.h"

static bool serverStarted = false;
bool botStarted = false;
static unsigned long lastReconnectAttempt = 0;

void setupWebServerRoutes() {
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
        server.sendHeader("Connection", "close");
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
      server.sendHeader("Connection", "close");
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
      server.sendHeader("Connection", "close");
      server.streamFile(file, "text/html");
      file.close();
    });
    server.on("/archive", HTTP_GET, handleArchiveList); // Генерирует HTML-страницу со списком всех архивных файлов (_graph.json).
    server.on("/view_logs", HTTP_GET, handleViewLogs);
    server.on("/clear_logs", HTTP_GET, handleClearLogs);
    server.on("/logs", HTTP_GET, handleLogs);           // Отдает сырой лог файл
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
}

void setupServices() {
    if (WiFi.status() == WL_CONNECTED) {
        static IPAddress lastIP;
        if (!WIFIENABLE || WiFi.localIP() != lastIP) {
            lastIP = WiFi.localIP();
            MYDEBUG_PRINT("Wi-Fi подключен! IP:");
            MYDEBUG_PRINTLN(lastIP);
            logEvent("Wi-Fi підключено! IP: " + lastIP.toString());

            // Настройка NTP для Киева
            configTime(TZ_INFO, "pool.ntp.org", "time.nist.gov", "time.google.com");
            logEvent("NTP час налаштовано.");

            client.setBufferSizes(1024, 512); 
            client.setInsecure();
            WIFIENABLE = 1;
        }

        if (!serverStarted) {
            server.begin();
            serverStarted = true;
            MYDEBUG_PRINTLN("HTTP server started");
        }

        if (!botStarted && strlen(botToken) > 40) {
            bot.updateToken(botToken);
            String statusMess = WORD_TITLE + String(settings.sp_structs[1].special) + NEW_STR;
            statusMess += WORD_IP + WiFi.localIP().toString() + NEW_STR;
            statusMess += GRAVE_ACCENT;
            bot.sendMessage(chatID, statusMess, "Markdown");
            botStarted = true;
            BOTENABLE = 1;
            MYDEBUG_PRINTLN("Telegram bot started");
        }
    }
}

void initWiFiManag(void){
    WiFiManager wifiManager;

    if(settings.sp_structs[0].special & 0x08){
      settings.sp_structs[0].special &= 0xF7;
      saveSetpoint();
      MYDEBUG_PRINTLN("Сброс настроек WiFi!");
      wifiManager.resetSettings();
    } 

    uint8_t tt = (settings.sp_structs[0].special & 0x03) * 60;
    if (tt == 0 && (settings.sp_structs[0].special & 0x03)) tt = 60; 

    MYDEBUG_PRINT("Устанавливаем таймаут для портала конфигурации: "); MYDEBUG_PRINTLN(tt);
    
    data[0] = 0b00111001; // C
    data[1] = 0b01011100; // o
    data[2] = 0b01010100; // n
    if(tt/100) data[3] = NUMBER_FONT[tt/100]; else data[3] = DEF;
    data[4] = NUMBER_FONT[(tt/10)%10];
    data[5] = NUMBER_FONT[tt%10];
    data[6] = 0b00111110; // U
    data[7] = 0b01110001; // F
    module.setDisplay(data, 8);
    
    wifiManager.setConfigPortalTimeout(tt);    
    
    setupWebServerRoutes(); // Регистрируем маршруты всегда

    if (!wifiManager.autoConnect("ClimatAP")) {
      MYDEBUG_PRINTLN("Не удалось подключиться (истек таймаут). Продолжаем работу в оффлайн-режиме.");
      data[3] = DEF; data[4] = DEF; data[5] = DEF;
      module.setDisplay(data, 8);
      delay(1000);
    } else {
        setupServices();

        data[3] = GR; data[4] = GR; data[5] = GR;
        module.setDisplay(data, 8);
        delay(1000);
    }
}

void handleWiFi(void) {
    if (WiFi.status() != WL_CONNECTED) {
        if (WIFIENABLE) {
            MYDEBUG_PRINTLN("Wi-Fi связь потеряна!");
            logEvent("Wi-Fi зв'язок втрачено!");
            WIFIENABLE = 0;
            BOTENABLE = 0;
            botStarted = false;
        }
        
        // Попытка переподключения каждые 2 минуты (быстрее чем 5)
        if (millis() - lastReconnectAttempt > 120000 || lastReconnectAttempt == 0) {
            lastReconnectAttempt = millis();
            MYDEBUG_PRINTLN("Попытка переподключения к Wi-Fi...");
            WiFi.begin(); 
        }
    } else {
        setupServices(); // Если подключено, убеждаемся что всё запущено
    }
}

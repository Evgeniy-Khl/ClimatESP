#include "main.h"
#include "server.h"
#include <vector>
#include <algorithm>

char wifiSsid[33] = "";
char wifiPass[65] = "";
bool apActive = false;
bool shouldReboot = false;
uint32_t rebootTimer = 0;

static bool serverStarted = false;
bool botStarted = false;
static unsigned long lastReconnectAttempt = 0;
static DNSServer dnsServer;

void handleScanWifi() {
    ESP.wdtFeed();
    int8_t n = WiFi.scanNetworks();
    ESP.wdtFeed();

    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    if (n > 0) {
        std::vector<int> indices(n);
        for (int i = 0; i < n; i++) indices[i] = i;
        std::sort(indices.begin(), indices.end(), [](int a, int b) {
            return WiFi.RSSI(a) > WiFi.RSSI(b);
        });

        std::vector<String> seenSSIDs;
        for (int i = 0; i < n; i++) {
            int idx = indices[i];
            String ssid = WiFi.SSID(idx);
            if (ssid.length() == 0) continue; // пропускаем скрытые

            // Дедупликация одинаковых SSID (оставляем с лучшим сигналом)
            bool duplicate = false;
            for (const String& s : seenSSIDs) {
                if (s == ssid) { duplicate = true; break; }
            }
            if (duplicate) continue;
            seenSSIDs.push_back(ssid);

            JsonObject item = array.add<JsonObject>();
            item["ssid"] = ssid;
            item["rssi"] = WiFi.RSSI(idx);
            item["enc"] = (WiFi.encryptionType(idx) != ENC_TYPE_NONE);
        }
    }
    WiFi.scanDelete();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void setupWebServerRoutes() {
    server.on("/scan_wifi", HTTP_GET, handleScanWifi);
    server.on("/", HTTP_GET, []() {
      mode = READDEFAULT; interval = INTERVAL_4000; tmrTelegramOff = 20;
      if (!LittleFS.exists("/index.html")) {
        MYDEBUG_PRINTLN("index.html not found");
      } else {
        File file = LittleFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "I can't open the index.html");
            return;
        }
        streamFileChunked(file, "text/html");
        file.close();
      }
    });
    server.on("/setup", HTTP_GET, []() {
      File file = LittleFS.open("/setup.html", "r");
      if (!file) {
          server.send(404, "text/plain", "File Not Found");
          return;
      }
      streamFileChunked(file, "text/html");
      file.close();
    });
    server.on("/table", HTTP_GET, []() {
      File file = LittleFS.open("/table.html", "r");
      if (!file) {
          server.send(404, "text/plain", "File Not Found");
          return;
      }
      streamFileChunked(file, "text/html");
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
    server.on("/reset_runaway", HTTP_ANY, []() {
        RUNAWAY = 0;
        stagnationTimer = 0;
        server.send(200, "text/plain", "OK");
    });
    server.onNotFound(notFoundHandler);
}

void setupServices() {
    if (WiFi.status() == WL_CONNECTED) {
        static IPAddress lastIP;
        if (!WIFIENABLE || WiFi.localIP() != lastIP) {
            lastIP = WiFi.localIP();
            MYDEBUG_PRINT("Wi-Fi подключен! IP: ");
            MYDEBUG_PRINTLN(lastIP);

            // Настройка NTP для Киева
            configTime(TZ_INFO, "pool.ntp.org", "time.nist.gov", "time.google.com");

            client.setBufferSizes(1024, 512); 
            client.setInsecure();
            WIFIENABLE = 1;
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

    if (!serverStarted) {
        server.begin();
        serverStarted = true;
        MYDEBUG_PRINTLN("HTTP server started");
    }
}

void startAccessPoint(void) {
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("Climat-AP");

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    apActive = true;

    if (!serverStarted) {
        server.begin();
        serverStarted = true;
        MYDEBUG_PRINTLN("HTTP server started in AP mode");
    }

    MYDEBUG_PRINT("Точка доступа запущена. IP: ");
    MYDEBUG_PRINTLN(WiFi.softAPIP());

    // Индикация на 7-сегментнике: "A P   O n"
    data[0] = 0b01110111; // A
    data[1] = 0b01110011; // P
    data[2] = DEF;
    data[3] = DEF;
    data[4] = DEF;
    data[5] = 0b01011100; // o
    data[6] = 0b01010100; // n
    data[7] = DEF;
    module.setDisplay(data, 8);
    delay(1000);
}

void initWiFiManag(void) {
    setupWebServerRoutes();

    if (settings.sp_structs[0].special & 0x08) {
        settings.sp_structs[0].special &= 0xF7;
        saveSetpoint();
        MYDEBUG_PRINTLN("Сброс настроек WiFi!");
        wifiSsid[0] = '\0';
        wifiPass[0] = '\0';
        if (LittleFS.exists("/config.json")) {
            File f = LittleFS.open("/config.json", "r");
            JsonDocument j;
            if (f) {
                deserializeJson(j, f);
                f.close();
            }
            j["ssid"] = "";
            j["pass"] = "";
            File fw = LittleFS.open("/config.json", "w");
            if (fw) {
                serializeJson(j, fw);
                fw.close();
            }
        }
    }

    if (strlen(wifiSsid) > 0) {
        MYDEBUG_PRINT("Подключение к Wi-Fi сети: ");
        MYDEBUG_PRINTLN(wifiSsid);

        // Индикация на 7-сегментнике: "C o n"
        data[0] = 0b00111001; // C
        data[1] = 0b01011100; // o
        data[2] = 0b01010100; // n
        data[3] = DEF; data[4] = DEF; data[5] = DEF;
        data[6] = 0b00111110; // U
        data[7] = 0b01110001; // F
        module.setDisplay(data, 8);

        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiSsid, wifiPass);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 24) {
            delay(500);
            ESP.wdtFeed();
            attempts++;
            if (attempts % 2 == 0) {
                data[4] = NUMBER_FONT[(attempts / 2) % 10];
                module.setDisplay(data, 8);
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            apActive = false;
            MYDEBUG_PRINTLN("Wi-Fi подключен к роутеру!");
            setupServices();

            data[3] = GR; data[4] = GR; data[5] = GR;
            module.setDisplay(data, 8);
            delay(1000);
            return;
        } else {
            MYDEBUG_PRINTLN("Не удалось подключиться к роутеру. Запуск автономной AP.");
            logEvent("Не вдалося підключитися до роутера. Перехід в режим автономної точки доступу.");
        }
    } else {
        MYDEBUG_PRINTLN("Настройки Wi-Fi отсутствуют. Запуск автономной AP.");
    }

    startAccessPoint();
}

void handleWiFi(void) {
    if (shouldReboot && (millis() - rebootTimer >= 1500)) {
        ESP.restart();
    }

    if (apActive) {
        dnsServer.processNextRequest();

        // Фоновая попытка подключиться к роутеру, если SSID задан
        if (strlen(wifiSsid) > 0) {
            if (millis() - lastReconnectAttempt > 120000 || lastReconnectAttempt == 0) {
                lastReconnectAttempt = millis();
                WiFi.mode(WIFI_AP_STA);
                WiFi.begin(wifiSsid, wifiPass);
            }
            if (WiFi.status() == WL_CONNECTED) {
                MYDEBUG_PRINTLN("Фоновое переподключение к роутеру успешно!");
                dnsServer.stop();
                WiFi.softAPdisconnect(true);
                WiFi.mode(WIFI_STA);
                apActive = false;
                setupServices();
            }
        }
    } else {
        if (WiFi.status() != WL_CONNECTED) {
            if (WIFIENABLE) {
                MYDEBUG_PRINTLN("Wi-Fi связь потеряна!");
                logEvent("Wi-Fi зв'язок втрачено!");
                WIFIENABLE = 0;
                BOTENABLE = 0;
                botStarted = false;
            }

            if (millis() - lastReconnectAttempt > 30000 || lastReconnectAttempt == 0) {
                lastReconnectAttempt = millis();
                MYDEBUG_PRINTLN("Попытка переподключения к Wi-Fi...");
                WiFi.begin(); 
            }

            static unsigned long disconnectStart = 0;
            if (disconnectStart == 0) disconnectStart = millis();
            if (millis() - disconnectStart > 120000 && !apActive) {
                MYDEBUG_PRINTLN("Роутер не отвечает более 2 минут. Активация резервной точки доступа.");
                startAccessPoint();
            }
        } else {
            setupServices();
        }
    }
}

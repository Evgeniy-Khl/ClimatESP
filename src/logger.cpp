#include "logger.h"
#include "main.h"

void logEvent(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    
    logEvent(String(buf));
}

void logEvent(const String& message) {
    String logLine = "";
    
    // Получаем текущее время
    if (RTCENABLE) {
        DateTime curr = rtc.now();
        char timeStr[24];
        snprintf(timeStr, sizeof(timeStr), "%02d.%02d.%04d %02d:%02d:%02d", 
                 curr.day(), curr.month(), curr.year(), curr.hour(), curr.minute(), curr.second());
        logLine += "[" + String(timeStr) + "] ";
    } else {
        logLine += "[" + String(millis() / 1000) + "s] ";
    }
    
    logLine += message + "\n";
    
    // Выводим в Serial для отладки
    #ifdef DEBUG
    Serial.print("LOG: " + logLine);
    #endif
    
    // Ротация логов при превышении 50 КБ
    if (LittleFS.exists("/system.log")) {
        File f = LittleFS.open("/system.log", "r");
        if (f) {
            size_t sz = f.size();
            f.close();
            if (sz > 50000) {
                #ifdef DEBUG
                Serial.println("System log exceeds 50KB, rotating...");
                #endif
                LittleFS.rename("/system.log", "/system.log.old");
                LittleFS.remove("/system.log");
            }
        }
    }
    
    // Запись в файл
    File logFile = LittleFS.open("/system.log", "a");
    if (logFile) {
        logFile.print(logLine);
        logFile.close();
    }
}

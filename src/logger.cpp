#include "logger.h"
#include "main.h"

bool fsMounted = false;

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
    DateTime curr;
    bool hasTime = false;
    
    // Получаем текущее время только если RTC инициализирован
    if (RTCENABLE) {
        curr = rtc.now();
        hasTime = true;
        char timeStr[25];
        snprintf(timeStr, sizeof(timeStr), "%02d.%02d.%04d %02d:%02d:%02d", 
                 curr.day(), curr.month(), curr.year(), curr.hour(), curr.minute(), curr.second());
        logLine += "[" + String(timeStr) + "] ";
    } else {
        logLine += "[" + String(millis() / 1000) + "s] ";
    }
    
    logLine += message + "\n";
    
    // Выводим в Serial для отладки
    #ifdef DEBUG
    if (Serial) {
        Serial.print("LOG: " + logLine);
    }
    #endif
    
    // Запись в файл производим только если файловая система смонтирована
    if (fsMounted) {
        // Имя файла по шаблону: /day_DD_MM_log.txt
        char logFilename[32];
        if (hasTime) {
            snprintf(logFilename, sizeof(logFilename), "/day_%02d_%02d_log.txt", curr.day(), curr.month());
        } else {
            snprintf(logFilename, sizeof(logFilename), "/day_01_01_log.txt");
        }
        
        // Ротация посуточного лога при превышении 50 КБ
        if (LittleFS.exists(logFilename)) {
            File f = LittleFS.open(logFilename, "r");
            if (f) {
                size_t sz = f.size();
                f.close();
                if (sz > 50000) {
                    #ifdef DEBUG
                    if (Serial) {
                        Serial.printf("Log file %s exceeds 50KB, cleaning up...\n", logFilename);
                    }
                    #endif
                    LittleFS.remove(logFilename);
                }
            }
        }
        
        // Запись в файл
        File logFile = LittleFS.open(logFilename, "a");
        if (logFile) {
            logFile.print(logLine);
            logFile.close();
        }
    }
}


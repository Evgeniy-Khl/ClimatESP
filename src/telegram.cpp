/*
Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/
#include <main.h>

extern char botToken[], chatID [];
extern MyTelegramBot bot;
extern bool shouldSaveConfig;

bool botSetup(){
  return bot.setMyCommands(MAIN_MENU);
}

/******************************************************************
* errors = 0x01   // ОШИБКА ДАТЧИКА 0  SENSOR_ERROR_VAL; SENSOR_FROZEN_VAL [E01]
* errors = 0x02   // ОШИБКА ДАТЧИКА 1  SENSOR_ERROR_VAL; SENSOR_FROZEN_VAL [E02]
* errors = 0x04   // ОТКЛОНЕНИЕ КАНАЛ 0 [E04]
* errors = 0x08   // ОТКЛОНЕНИЕ КАНАЛ 1 [E08]
* errors = 0x10   // отказ одного из двух датчиков температуры
* errors = 0x20   // отказ вспомогательного датчика температуры
* errors = 0x80   // ПЕРЕГРЕВ СИМИСТОРА ! [ПРГ] 
*******************************************************************/
void sendErrMessages(int err){
  char buffer[512];
  int pos = 0;
  
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_TITLE);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("Клімат-5.25"));
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)ID_TITLE);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("1\n\n"));

  if(errorsFlag.value & 1) pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)SENSOR_ERROR_1);
  if(errorsFlag.value & 2) pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)SENSOR_ERROR_2);
  if(errorsFlag.value & 4){
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)SENSOR_ERROR_4);
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_AIR);
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%.1f\n"), (float)ds[0].pvT/10);
  }
  if(errorsFlag.value & 8){
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)SENSOR_ERROR_8);
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_PRODUCT);
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%.1f\n"), (float)ds[1].pvT/10);
  }
  if(RUNAWAY) pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)RUNAWAY_ERROR);
  
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("```"));
  
  bot.sendMessage(chatID, buffer, "Markdown");
}

void sendStatus(){
  char buffer[512];
  int pos = 0;

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_TITLE);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("Клімат-5.25"));
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)ID_TITLE);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("1\n\n"));

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_AIR);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%.1f°C\n"), (float)ds[0].pvT/10);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_PRODUCT);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%.1f°C\n"), (float)ds[1].pvT/10);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_HUMIDITY);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d%%\n"), pvRH);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_HEATING);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d%%\n"), pctHeater);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_DAMPER);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d%%\n"), pvFlap);

  if (settings.sp_structs[1].state > 0) {
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("- доба інкубації: %d\n"), countDays + 1);
  }
  if (settings.sp_structs[0].timer == 0 && settings.sp_structs[1].timer > 0) {
    pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("- таймер: %s (%d год.)\n"), TURN == PCF_ON ? "УВІМК" : "ВИМК", pvTimer);
  }

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_MISTAKES);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d\n"), errorsFlag.value);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("```"));
  
  bot.sendMessage(chatID, buffer, "Markdown");
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
    MYDEBUG_PRINTLN("handleNewMessages...");
    for (int i = 0; i < numNewMessages; i++) {
        // Chat id of the requester
        if (strcmp(bot.messages[i].chat_id.c_str(), chatID) != 0){
          bot.sendMessage(bot.messages[i].chat_id, "Unauthorized user", "");
          continue;
        }
        
        MYDEBUG_PRINTLN("received message: ");
        MYDEBUG_PRINTLN(bot.messages[i].text.c_str());
    
        if (bot.messages[i].text == TXT_START) {
          if (settings.sp_structs[0].timer == 0 && settings.sp_structs[1].timer > 0 && TURN == PCF_OFF) {
            TURN = PCF_ON;
            pvTimer = settings.sp_structs[1].timer;
            oneShotMinutes = 0;
            TURNSECOND = OFF;
            writePCF8574(portOut.value);
            uint32_t currentSec = (RTCENABLE) ? rtc.now().unixtime() : now.unixtime();
            saveOneShotEndTime(currentSec + (uint32_t)pvTimer * 3600UL);
            logEvent("Таймер: увімкнено на %d год.", pvTimer);
            char msg[64];
            snprintf_P(msg, sizeof(msg), PSTR("Таймер увімкнено на %d год."), pvTimer);
            bot.sendMessage(chatID, msg, "");
          }
        }
        else if (bot.messages[i].text == TXT_STOP) {
          if (settings.sp_structs[0].timer == 0 && settings.sp_structs[1].timer > 0 && TURN == PCF_ON) {
            logEvent("Таймер: вимкнено користувачем");
            clearOneShotEndTime();
            ds[0].deviation = 0;
            ds[1].deviation = 0;
            TURN = PCF_OFF;
            oneShotMinutes = 0;
            pvTimer = 0;
            TURNSECOND = OFF;
            writePCF8574(portOut.value);
            bot.sendMessage(chatID, "Таймер вимкнено.", "");
          }
        }
        else if (bot.messages[i].text == TXT_STATUS) {
          sendStatus();
        }
    }
}

void checkTelegram() {
    if (!BOTENABLE || !WIFIENABLE) return;
    if (tmrTelegramOff > 0) return;
    if (ESP.getFreeHeap() < 18000) return;

    static unsigned long lastCheck = 0;
    unsigned long now = millis();
    if (now - lastCheck < 5000) return;
    lastCheck = now;

    ESP.wdtFeed();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    ESP.wdtFeed();

    if (numNewMessages > 0) {
        handleNewMessages(numNewMessages);
    }
}
  
//callback notifying us of the need to save config
void saveConfigCallback() {
    MYDEBUG_PRINTLN("Should save config");
    shouldSaveConfig = true;
}
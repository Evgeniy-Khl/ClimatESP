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
* errors = 0x01   // ОШИБКА ДАТЧИКА 0  199-потерян; 66,0-завис [E01]
* errors = 0x02   // ОШИБКА ДАТЧИКА 1  199-потерян; 66,0-завис [E02]
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
  if(RUNAWAY_ERR) pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)RUNAWAY_ERROR);
  
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
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%.1f\n"), (float)ds[0].pvT/10);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_PRODUCT);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%.1f\n"), (float)ds[1].pvT/10);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_HUMIDITY);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d%%\n"), pvRH);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_HEATING);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d%%\n"), pctHeater);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_DAMPER);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d%%\n"), pvFlap);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, (PGM_P)WORD_MISTAKES);
  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("%d\n"), errorsFlag.value);

  pos += snprintf_P(buffer + pos, sizeof(buffer) - pos, PSTR("```"));
  
  bot.sendMessage(chatID, buffer, "Markdown");
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
    MYDEBUG_PRINTLN("handleNewMessages...");
    for (int i=0; i<numNewMessages; i++) {
        // Chat id of the requester
        if (strcmp(bot.messages[i].chat_id.c_str(), chatID) != 0){
          bot.sendMessage(bot.messages[i].chat_id, "Unauthorized user", "");
          continue;
        }
        
        const char* text = bot.messages[i].text.c_str();
        MYDEBUG_PRINTLN("received message: ");
        MYDEBUG_PRINTLN(text);
    
        if (bot.messages[i].text == TXT_START) {
          char welcome[256];
          snprintf_P(welcome, sizeof(welcome), PSTR("Welcome, %s.\nUse the following commands to control your outputs.\n\n/status - Get status\n/options - Show options"), bot.messages[i].from_name.c_str());
          bot.sendMessage(chatID, welcome, "");
        }
        if (bot.messages[i].text == TXT_OPTIONS){
          const char* keyboardJson = "[[{ \"text\" : \"Go to Graviton\", \"url\" : \"https://graviton.com.ua/ua/\" }],[{ \"text\" : \"Send\", \"callback_data\" : \"/start\" }]]";
          bot.sendMessageWithInlineKeyboard(chatID, "Choose from one of the following options", "", keyboardJson);
        }
        if (bot.messages[i].text == TXT_STATUS) sendStatus();
    }
}
  
//callback notifying us of the need to save config
void saveConfigCallback() {
    MYDEBUG_PRINTLN("Should save config");
    shouldSaveConfig = true;
}
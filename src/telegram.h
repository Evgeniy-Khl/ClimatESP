/*
Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/
#ifndef TELEGRAM_H
#define TELEGRAM_H

bool botSetup();
void sendStatus();
void sendErrMessages(int err);
void saveConfigCallback();
void handleNewMessages(int numNewMessages);
#endif //TELEGRAM_H

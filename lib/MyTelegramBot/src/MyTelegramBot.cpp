/*
   
 */

#include "MyTelegramBot.h"

#define TELEGRAM_DEBUG

#define ZERO_COPY(STR)    ((char*)STR.c_str())
#define BOT_CMD(STR)      buildCommand(F(STR))

MyTelegramBot::MyTelegramBot(const String& token, Client &client) {
  updateToken(token);
  this->client = &client;
}

void MyTelegramBot::updateToken(const String& token) {
  _token = token;
}

String MyTelegramBot::getToken() {
  return _token;
}

String MyTelegramBot::buildCommand(const String& cmd) {
  String command;

  command += F("bot");
  command += _token;
  command += F("/");
  command += cmd;

  return command;
}

String MyTelegramBot::sendGetToTelegram(const String& command) {
  String body, headers;
  
  // Connect with api.telegram.org if not already connected
  if (!client->connected()) {
    #ifdef TELEGRAM_DEBUG  
        Serial.println(F("sendGetToTelegram()->[BOT]Connecting to server"));
    #endif
    if (!client->connect(TELEGRAM_HOST, TELEGRAM_SSL_PORT)) {
      #ifdef TELEGRAM_DEBUG  
        Serial.println(F("[BOT]Conection error"));
      #endif
    }
  }
  if (client->connected()) {

    #ifdef TELEGRAM_DEBUG  
        Serial.println("sending: " + command);
    #endif  

    client->print(F("GET /"));
    client->print(command);
    client->println(F(" HTTP/1.1"));
    client->println(F("Host:" TELEGRAM_HOST));
    client->println(F("Accept: application/json"));
    client->println(F("Cache-Control: no-cache"));
    client->println();

    readHTTPAnswer(body, headers);
  }

  return body;
}

bool MyTelegramBot::readHTTPAnswer(String &body, String &headers) {
  int ch_count = 0;
  unsigned long now = millis();
  bool finishedHeaders = false;
  bool currentLineIsBlank = true;
  bool responseReceived = false;

  while (millis() - now < longPoll * 1000 + waitForResponse) {
    while (client->available()) {
      char c = client->read();
      responseReceived = true;

      if (!finishedHeaders) {
        if (currentLineIsBlank && c == '\n') {
          finishedHeaders = true;
        } else {
          headers += c;
        }
      } else {
        if (ch_count < maxMessageLength) {
          body += c;
          ch_count++;
        }
      }

      if (c == '\n') currentLineIsBlank = true;
      else if (c != '\r') currentLineIsBlank = false;
    }

    if (responseReceived) {
      #ifdef TELEGRAM_DEBUG  
        Serial.println();
        Serial.println("readHTTPAnswer():" + body);
        Serial.println();
      #endif
      break;
    }
  }
  return responseReceived;
}

String MyTelegramBot::sendPostToTelegram(const String& command, JsonObject payload) {

  String body;
  String headers;

  // Connect with api.telegram.org if not already connected
  if (!client->connected()) {
    #ifdef TELEGRAM_DEBUG  
        Serial.println(F("sendPostToTelegram()->[BOT Client]Connecting to server...."));
    #endif
    if (!client->connect(TELEGRAM_HOST, TELEGRAM_SSL_PORT)) {
      #ifdef TELEGRAM_DEBUG  
        Serial.println(F("[BOT Client]Conection error"));
      #endif
    }
  }
  if (client->connected()) {
    // POST URI
    client->print(F("POST /"));
    client->print(command);
    client->println(F(" HTTP/1.1"));
    // Host header
    client->println(F("Host:" TELEGRAM_HOST));
    // JSON content type
    client->println(F("Content-Type: application/json"));

    // Content length
    int length = measureJson(payload);
    client->print(F("Content-Length:"));
    client->println(length);
    // End of headers
    client->println();
    // POST message body
    String out;
    serializeJson(payload, out);
    
    client->println(out);
    #ifdef TELEGRAM_DEBUG
        Serial.println(String("Posting:") + out);
    #endif

    readHTTPAnswer(body, headers);
  }

  return body;
}

bool MyTelegramBot::getMe() {
  String response = sendGetToTelegram(BOT_CMD("getMe")); // receive reply from telegram.org
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, ZERO_COPY(response));
  closeClient();

  if (!error) {
    if (doc["result"].is<JsonObject>()) {
      name = doc["result"]["first_name"].as<String>();
      userName = doc["result"]["username"].as<String>();
      return true;
    }
  }

  return false;
}

/*********************************************************************************
 * SetMyCommands - Update the command list of the bot on the telegram server     *
 * (Argument to pass: Serialied array of BotCommand)                             *
 * CAUTION: All commands must be lower-case                                      *
 * Returns true, if the command list was updated successfully                    *
 ********************************************************************************/
bool MyTelegramBot::setMyCommands(const String& commandArray) {
  JsonDocument payload;
  payload["commands"] = serialized(commandArray);
  bool sent = false;
  String response = "";
  #if defined(_debug)
  Serial.println(F("sendSetMyCommands: SEND Post /setMyCommands"));
  #endif  // defined(_debug)
  unsigned long sttime = millis();

  while (millis() - sttime < 8000ul) { // loop for a while to send the message
    response = sendPostToTelegram(BOT_CMD("setMyCommands"), payload.as<JsonObject>());
    #ifdef _debug  
    Serial.println("setMyCommands response" + response);
    #endif
    sent = checkForOkResponse(response);
    if (sent) break;
  }

  closeClient();
  return sent;
}


/***************************************************************
 * GetUpdates - function to receive messages from telegram *
 * (Argument to pass: the last+1 message to read)             *
 * Returns the number of new messages           *
 ***************************************************************/
int MyTelegramBot::getUpdates(long offset) {

  #ifdef TELEGRAM_DEBUG  
    Serial.println(F("GET Update Messages"));
  #endif
  String command = BOT_CMD("getUpdates?offset=");
  command += offset;
  command += F("&limit=");
  command += HANDLE_MESSAGES;

  if (longPoll > 0) {
    command += F("&timeout=");
    command += String(longPoll);
  }
  String response = sendGetToTelegram(command); // receive reply from telegram.org

  if (response == "") {
    #ifdef TELEGRAM_DEBUG  
        Serial.println(F("Received empty string in response!"));
    #endif
    // close the client as there's nothing to do with an empty string
    // closeClient();
    return 0;
  } else {
    #ifdef TELEGRAM_DEBUG  
      Serial.print(F("incoming message length "));
      Serial.println(response.length());
      Serial.println(F("Creating DynamicJsonBuffer"));
    #endif

    // Parse response into Json object
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, ZERO_COPY(response));
      
    if (!error) {
      #ifdef TELEGRAM_DEBUG  
        Serial.print(F("GetUpdates parsed jsonObj: "));
        serializeJson(doc, Serial);
        Serial.println();
      #endif
      if (doc["result"].is<JsonArray>()) {
        int resultArrayLength = doc["result"].size();
        if (resultArrayLength > 0) {
          int newMessageIndex = 0;
          // Step through all results
          for (int i = 0; i < resultArrayLength; i++) {
            JsonObject result = doc["result"][i];
            #ifdef TELEGRAM_DEBUG  
              Serial.print(F("Parsed result: "));
              serializeJson(result, Serial);
              Serial.println();
            #endif
            if (processResult(result, newMessageIndex)) newMessageIndex++;
          }
          // We will keep the client open because there may be a response to be
          // given
          return newMessageIndex;
        } else {
          #ifdef TELEGRAM_DEBUG  
            Serial.println(F("no new messages"));
          #endif
        }
      } else {
        #ifdef TELEGRAM_DEBUG  
            Serial.println(F("Response contained no 'result'"));
        #endif
      }
    } else { // Parsing failed
      if (response.length() < 2) { // Too short a message. Maybe a connection issue
        #ifdef TELEGRAM_DEBUG  
            Serial.println(F("Parsing error: Message too short"));
        #endif
      } else {
        // Buffer may not be big enough, increase buffer or reduce max number of
        // messages
        #ifdef TELEGRAM_DEBUG 
            Serial.print(F("Failed to parse update, the message could be too "
                           "big for the buffer. Error code: "));
            Serial.println(error.c_str()); // debug print of parsing error
        #endif     
      }
    }
    // Close the client as no response is to be given
    // closeClient();
    return 0;
  }
}

bool MyTelegramBot::processResult(JsonObject result, int messageIndex) {
  int update_id = result["update_id"];
  // Check if we have already dealt with this message (this shouldn't happen!)
  if (last_message_received != update_id) {
      last_message_received = update_id;
      messages[messageIndex].update_id = update_id;
      messages[messageIndex].text = F("");
      messages[messageIndex].from_id = F("");
      messages[messageIndex].from_name = F("");
      messages[messageIndex].longitude = 0;
      messages[messageIndex].latitude = 0;
      messages[messageIndex].reply_to_message_id = 0;
      messages[messageIndex].reply_to_text = F("");
      messages[messageIndex].query_id = F("");

      if (result["message"].is<JsonObject>()) {
          JsonObject message = result["message"];
          messages[messageIndex].type = F("message");
          messages[messageIndex].from_id = message["from"]["id"].as<String>();
          messages[messageIndex].from_name = message["from"]["first_name"].as<String>();
          messages[messageIndex].date = message["date"].as<String>();
          messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
          messages[messageIndex].chat_title = message["chat"]["title"].as<String>();
          messages[messageIndex].hasDocument = false;
          messages[messageIndex].message_id = message["message_id"].as<int>();  // added message id

          if (message["text"].is<String>()) {
              messages[messageIndex].text = message["text"].as<String>();
          } else if (message["location"].is<JsonObject>()) {
              messages[messageIndex].longitude = message["location"]["longitude"].as<float>();
              messages[messageIndex].latitude = message["location"]["latitude"].as<float>();
          } else if (message["document"].is<JsonObject>()) {
              String file_id = message["document"]["file_id"].as<String>();
              messages[messageIndex].file_caption = message["caption"].as<String>();
              messages[messageIndex].file_name = message["document"]["file_name"].as<String>();
              if (getFile(messages[messageIndex].file_path, messages[messageIndex].file_size, file_id)) {
                  messages[messageIndex].hasDocument = true;
              } else {
                  messages[messageIndex].hasDocument = false;
              }
          }
          if (message["reply_to_message"].is<JsonObject>()) {
              messages[messageIndex].reply_to_message_id = message["reply_to_message"]["message_id"];
              messages[messageIndex].reply_to_text = message["reply_to_message"]["text"].as<String>();
          }

      } else if (result["channel_post"].is<JsonObject>()) {
          JsonObject message = result["channel_post"];
          messages[messageIndex].type = F("channel_post");
          messages[messageIndex].text = message["text"].as<String>();
          messages[messageIndex].date = message["date"].as<String>();
          messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
          messages[messageIndex].chat_title = message["chat"]["title"].as<String>();
          messages[messageIndex].message_id = message["message_id"].as<int>();  // added message id

      } else if (result["callback_query"].is<JsonObject>()) {
          JsonObject message = result["callback_query"];
          messages[messageIndex].type = F("callback_query");
          messages[messageIndex].from_id = message["from"]["id"].as<String>();
          messages[messageIndex].from_name = message["from"]["first_name"].as<String>();
          messages[messageIndex].text = message["data"].as<String>();
          messages[messageIndex].date = message["date"].as<String>();
          messages[messageIndex].chat_id = message["message"]["chat"]["id"].as<String>();
          messages[messageIndex].reply_to_text = message["message"]["text"].as<String>();
          messages[messageIndex].chat_title = F("");
          messages[messageIndex].query_id = message["id"].as<String>();
          messages[messageIndex].message_id = message["message"]["message_id"].as<int>();  // added message id

      } else if (result["edited_message"].is<JsonObject>()) {
          JsonObject message = result["edited_message"];
          messages[messageIndex].type = F("edited_message");
          messages[messageIndex].from_id = message["from"]["id"].as<String>();
          messages[messageIndex].from_name = message["from"]["first_name"].as<String>();
          messages[messageIndex].date = message["date"].as<String>();
          messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
          messages[messageIndex].chat_title = message["chat"]["title"].as<String>();
          messages[messageIndex].message_id = message["message_id"].as<int>();  // added message id

          if (message["text"].is<String>()) {
              messages[messageIndex].text = message["text"].as<String>();
          } else if (message["location"].is<JsonObject>()) {
              messages[messageIndex].longitude = message["location"]["longitude"].as<float>();
              messages[messageIndex].latitude = message["location"]["latitude"].as<float>();
          }
      }
      return true;
  }
  return false;
}

/***********************************************************************
 * SendMessage - function to send message to telegram                  *
 * (Arguments to pass: chat_id, text to transmit and markup(optional)) *
 ***********************************************************************/
bool MyTelegramBot::sendSimpleMessage(const String& chat_id, const String& text,
                                             const String& parse_mode) {

  bool sent = false;
  #ifdef TELEGRAM_DEBUG  
    Serial.println(F("sendSimpleMessage: SEND Simple Message"));
  #endif
  unsigned long sttime = millis();

  if (text != "") {
    while (millis() - sttime < 8000ul) { // loop for a while to send the message
      String command = BOT_CMD("sendMessage?chat_id=");
      command += chat_id;
      command += F("&text=");
      command += text;
      command += F("&parse_mode=");
      command += parse_mode;
      String response = sendGetToTelegram(command);
      #ifdef TELEGRAM_DEBUG  
        Serial.println(response);
      #endif
      sent = checkForOkResponse(response);
      if (sent) break;
    }
  }
  closeClient();
  return sent;
}

bool MyTelegramBot::sendMessage(const String& chat_id, const String& text,
                                       const String& parse_mode, int message_id) { // added message_id

  JsonDocument payload;
  payload["chat_id"] = chat_id;
  payload["text"] = text;

  if (message_id != 0)
    payload["message_id"] = message_id; // added message_id

  if (parse_mode != "")
    payload["parse_mode"] = parse_mode;

  return sendPostMessage(payload.as<JsonObject>(), message_id); // if message id == 0 then edit is false, else edit is true
}

bool MyTelegramBot::sendMessageWithReplyKeyboard(
    const String& chat_id, const String& text, const String& parse_mode, const String& keyboard,
    bool resize, bool oneTime, bool selective) {
    
  JsonDocument payload;
  payload["chat_id"] = chat_id;
  payload["text"] = text;

  if (parse_mode != "") payload["parse_mode"] = parse_mode;

  JsonObject replyMarkup = payload["reply_markup"].to<JsonObject>();
    
  replyMarkup["keyboard"] = serialized(keyboard);

  // Telegram defaults these values to false, so to decrease the size of the
  // payload we will only send them if needed
  if (resize)
    replyMarkup["resize_keyboard"] = resize;

  if (oneTime)
    replyMarkup["one_time_keyboard"] = oneTime;

  if (selective)
    replyMarkup["selective"] = selective;

  return sendPostMessage(payload.as<JsonObject>());
}

bool MyTelegramBot::sendMessageWithInlineKeyboard(const String& chat_id,
                                                         const String& text,
                                                         const String& parse_mode,
                                                         const String& keyboard,
                                                         int message_id) {   // added message_id

  JsonDocument payload;
  payload["chat_id"] = chat_id;
  payload["text"] = text;

  if (message_id != 0)
    payload["message_id"] = message_id; // added message_id
    
  if (parse_mode != "") payload["parse_mode"] = parse_mode;

  JsonObject replyMarkup = payload["reply_markup"].to<JsonObject>();
  replyMarkup["inline_keyboard"] = serialized(keyboard);
  return sendPostMessage(payload.as<JsonObject>(), message_id); // if message id == 0 then edit is false, else edit is true
}

/***********************************************************************
 * SendPostMessage - function to send message to telegram              *
 * (Arguments to pass: chat_id, text to transmit and markup(optional)) *
 ***********************************************************************/
bool MyTelegramBot::sendPostMessage(JsonObject payload, bool edit) { // added message_id

  bool sent = false;
  #ifdef TELEGRAM_DEBUG 
    Serial.print(F("sendPostMessage: SEND Post Message: "));
    serializeJson(payload, Serial);
    Serial.println();
  #endif 
  unsigned long sttime = millis();

  if (payload["text"].is<String>()) {
    while (millis() < sttime + 8000) { // loop for a while to send the message
        String response = sendPostToTelegram((edit ? BOT_CMD("editMessageText") : BOT_CMD("sendMessage")), payload); // if edit is true we send a editMessageText CMD
        #ifdef TELEGRAM_DEBUG  
        Serial.println("sendPostMessage():" + response);
        #endif
        sent = checkForOkResponse(response);
        if (sent) break;
    }
  }

  // closeClient();
  return sent;
}

/* bool MyTelegramBot::checkForOkResponse(const String& response) {
  int last_id;
  DynamicJsonDocument doc(response.length());
  deserializeJson(doc, response);

  // Save last sent message_id
  last_id = doc["result"]["message_id"];
  if (last_id > 0) last_sent_message_id = last_id;

  return doc["ok"] | false;  // default is false, but this is more explicit and clear
} */

bool MyTelegramBot::checkForOkResponse(const String& response) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);

  // Проверка на ошибки десериализации
  if (error) {
      Serial.print(F("Failed to parse JSON: "));
      Serial.println(error.f_str());
      return false; // Если произошла ошибка, возвращаем false
  }

  // Проверяем, что массив result не пустой
  if (doc["ok"] && doc["result"].size() > 0) {
      // Сохраняем last_sent_message_id
      last_sent_message_id = doc["result"][0]["message_id"] | 0; // Установим в 0, если message_id отсутствует
      // #ifdef TELEGRAM_DEBUG  
      //   Serial.println("last_sent_message_id:"+String(last_sent_message_id));
      // #endif
      return true; // Возвращаем true, если всё в порядке
  }

  return false; // Если ok = false или result пустой
}

bool MyTelegramBot::sendChatAction(const String& chat_id, const String& text) {

  bool sent = false;
  #ifdef TELEGRAM_DEBUG  
    Serial.println(F("SEND Chat Action Message"));
  #endif
  unsigned long sttime = millis();

  if (text != "") {
    while (millis() - sttime < 8000ul) { // loop for a while to send the message
      String command = BOT_CMD("sendChatAction?chat_id=");
      command += chat_id;
      command += F("&action=");
      command += text;

      String response = sendGetToTelegram(command);

      #ifdef TELEGRAM_DEBUG  
        Serial.println(response);
      #endif
      sent = checkForOkResponse(response);

      if (sent) break;
      
    }
  }

  closeClient();
  return sent;
}

void MyTelegramBot::closeClient() {
  if (client->connected()) {
    #ifdef TELEGRAM_DEBUG  
        Serial.println(F("Closing client"));
    #endif
    client->stop();
  }
}

bool MyTelegramBot::getFile(String& file_path, long& file_size, const String& file_id)
{
  String command = BOT_CMD("getFile?file_id=");
  command += file_id;
  String response = sendGetToTelegram(command); // receive reply from telegram.org
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, ZERO_COPY(response));
  closeClient();

  if (!error) {
    if (doc["result"].is<JsonObject>()) {
        file_path = F("https://api.telegram.org/file/");
        file_path += buildCommand(doc["result"]["file_path"]);
        file_size = doc["result"]["file_size"].as<long>();
        return true;
    }
}
  return false;
}

bool MyTelegramBot::answerCallbackQuery(const String &query_id, const String &text, bool show_alert, const String &url, int cache_time) {
  JsonDocument payload;

  payload["callback_query_id"] = query_id;
  payload["show_alert"] = show_alert;
  payload["cache_time"] = cache_time;

  if (text.length() > 0) payload["text"] = text;
  if (url.length() > 0) payload["url"] = url;

  String response = sendPostToTelegram(BOT_CMD("answerCallbackQuery"), payload.as<JsonObject>());
  #ifdef _debug  
     Serial.print(F("answerCallbackQuery response:"));
     Serial.println(response);
  #endif
  bool answer = checkForOkResponse(response);
  closeClient();
  return answer;
}

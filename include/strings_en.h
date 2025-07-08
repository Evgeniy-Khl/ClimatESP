// strings_en.h
#ifndef STRINGS_EN_H
#define STRINGS_EN_H

#include <Arduino.h>

#define MAIN_MENU           F("[ \
    {\"command\":\"help\",  \"description\":\"Get help using the bot\"}, \
    {\"command\":\"start\", \"description\":\"A message is sent when you open a chat with a bot\"}, \
    {\"command\":\"status\",\"description\":\"Current device status.\"} \
    ]")
#define WORD_TITLE          F("```\n Climate-5.")
#define ID_TITLE            F("  ID:")
#define WORD_AIR            F("- air: ")
#define WORD_PRODUCT        F("- product: ")
#define WORD_HUMIDITY       F("- humidity:  ")
#define WORD_HEATING        F("- heating: ")
#define WORD_DAMPER         F("- damper: ")
#define WORD_TOTAL          F("- total: ")
#define WORD_MISTAKES       F("- mistakes: ")
#define WORD_WARNING        F("- warning: ")
#define WORD_DAYS           F("days")
#define TXT_START           F("/start")
#define TXT_OPTIONS         F("/options")
#define TXT_STATUS          F("/status")

#define STR_CONNECTING      F("Connecting to server")
#define STR_DATA_RETRIEVED  F("Data retrieved successfully")
#define STR_ERROR           F("An error occurred")
#define SENSOR_ERROR_1      F("SENSOR ERROR #1\n")
#define SENSOR_ERROR_2      F("SENSOR ERROR #2\n")
#define SENSOR_ERROR_4      F("TEMPERATURE DEVIATION #1\n")
#define SENSOR_ERROR_8      F("TEMPERATURE DEVIATION #2\n")
#define NEW_STR             F("\n")

#endif // STRINGS_EN_H
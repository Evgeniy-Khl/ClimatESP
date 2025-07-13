// strings_ua.h
#ifndef STRINGS_UA_H
#define STRINGS_UA_H

#include <Arduino.h>

#define MAIN_MENU           F("[ \
    {\"command\":\"help\",  \"description\":\"Отримайте допомогу з використанням бота\"}, \
    {\"command\":\"start\", \"description\":\"Повідомлення яке надсилається, коли ви відкриваєте чат із ботом\"}, \
    {\"command\":\"status\",\"description\":\"Поточний стан пристрою.\"} \
    ]")
#define WORD_TITLE          F("```\n Клімат-5.")
#define ID_TITLE            F("  ID:")
#define WORD_AIR            F("- повітря: ")
#define WORD_PRODUCT        F("- продукт: ")
#define WORD_HUMIDITY       F("- вологість:  ")
#define WORD_HEATING        F("- підігрів: ")
#define WORD_DAMPER         F("- заслінки: ")
#define WORD_TOTAL          F("- інкубація: ")
#define WORD_MISTAKES       F("- помилки: ")
#define WORD_WARNING        F("- попередження: ")
#define WORD_DAYS           F("діб")
#define TXT_START           F("/start")
#define TXT_OPTIONS         F("/options")
#define TXT_STATUS          F("/status")

#define STR_CONNECTING      F("Підключення до сервера")
#define STR_DATA_RETRIEVED  F("Дані успішно отримані")
#define STR_ERROR           F("Сталася помилка")
#define SENSOR_ERROR_1      F("ПОМИЛКА ДАТЧИКА №1\n")
#define SENSOR_ERROR_2      F("ПОМИЛКА ДАТЧИКА №2\n")
#define SENSOR_ERROR_4      F("ВІДХИЛЕННЯ ТЕМПЕРАТУРИ №1\n")
#define SENSOR_ERROR_8      F("ВІДХИЛЕННЯ ТЕМПЕРАТУРИ №2\n")
#define NEW_STR             F("\n")

#endif // STRINGS_UA_H
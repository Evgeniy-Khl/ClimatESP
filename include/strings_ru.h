// strings_ru.h
#ifndef STRINGS_RU_H
#define STRINGS_RU_H

#include <Arduino.h>

#define MAIN_MENU           F("[ \
    {\"command\":\"help\",  \"description\":\"Получите помощь в использовании бота\"}, \
    {\"command\":\"start\", \"description\":\"Сообщение отправляется, когда вы открываете чат с ботом\"}, \
    {\"command\":\"status\",\"description\":\"Текущее состояние устройства.\"} \
    ]")
#define WORD_TITLE          F("```\n Климат-5.")
#define ID_TITLE            F("  ID:")
#define WORD_AIR            F("- воздух: ")
#define WORD_PRODUCT        F("- продукт: ")
#define WORD_HUMIDITY       F("- влажность:  ")
#define WORD_HEATING        F("- подогрев: ")
#define WORD_DAMPER         F("- заслонки: ")
#define WORD_TOTAL          F("- инкубация: ")
#define WORD_MISTAKES       F("- ошибки: ")
#define WORD_WARNING        F("- предупреждения: ")
#define WORD_DAYS           F("дней")
#define TXT_START           F("/start")
#define TXT_OPTIONS         F("/options")
#define TXT_STATUS          F("/status")

#define STR_CONNECTING      F("Подключение к серверу")
#define STR_DATA_RETRIEVED  F("Данные успешно получены")
#define STR_ERROR           F("Произошла ошибка")
#define SENSOR_ERROR_1      F("SENSOR ERROR #1\n")
#define SENSOR_ERROR_2      F("SENSOR ERROR #2\n")
#define SENSOR_ERROR_4      F("TEMPERATURE DEVIATION #1\n")
#define SENSOR_ERROR_8      F("TEMPERATURE DEVIATION #2\n")
#define NEW_STR             F("\n")

#endif // STRINGS_RU_H
#include "main.h"


// Пин, к которому подключен информационный вывод (DQ) датчика DS18B20

uint8_t numberOfDevices, errDevice[MAX_DEVICE];

// Адрес PCF8574. Может быть разным в зависимости от конфигурации A0, A1, A2.
// Стандартные адреса: 0x20-0x27 для PCF8574 и 0x38-0x3F для PCF8574A.
// Уточните адрес вашего модуля. Часто по умолчанию 0x27 или 0x3F.
#define PCF8574_ADDRESS 0x27 // Замените на ваш адрес, если необходимо
long lastMsg = 0, number = 0;
//---------------------------------
char displStr[50];
int16_t resetDispl, displOff = DISPLAYOFF, pvWait, pvVenting;
// Глобальный массив указателей, который будет доступен всем функциям
const char* keyLabel[15];
uint16_t keyColor[15];
bool newDispl = true, newTxt = true;
uint16_t xpos, ypos, txt_height, t_x = 0, t_y = 0; // To store the touch coordinates;
uint8_t displNum=0, seconds=0, pwTriac, pvTimer, pvFlap, beepOn;
float editValue;
// spT spRH timer alarm coolOn coolOff aeration flapLimit state service pulse mode extendMode Kp Ki Kd
//---------------------------------
Ds ds[2] = {{350,0},{280,0}};
int8_t dpv1 = 2;
float flT0=350, dpv0;
//---------------------------------
GrafDispl grafDispl[2] = {
    { 80,80,80, 0, 0},    // Инициализация grafDispl[0]
    {240,80,80, 0, 0},    // Инициализация grafDispl[1]
};
union Byte portOut;
union Byte errors;
union Byte portFlag;
//************************************************************************************************** */
#define FLPCLOSE 9
#define FLPOPEN  24
// РЕКОМЕНДУЕМЫЙ СПОСОБ:
SpUnion settings = {
    .sp_structs = { // Явно говорим, что инициализируем поле sp_structs
        { // Элемент sp_structs[0]
            .spT = 350,             // завдання у грд.Цесія #1 = 350
            .spRH = 1,              // завдання у відсотках (ПОДСТРОЙКА HIH) = 0
            .alarm = 2,             // аварійне відхилення #1 = 5
            .coolOn = 3,            // знижувач увімкнути #1 = 3
            .coolOff = 4,           // знижувач вимкнути #1 = 1
            .timer = 5,             // лотки увім. = 60
            .aeration = 6,          // провітр.пауза ПАУЗА ПРОВЕТРИВАНИЯ (минут) = 10
            .auxiliary = 7,         // допоміж. увім. = ?
            .state = 8,             // заслінка полож. = 0 - закрита; 100 - відкрита
            .flapLimit = FLPCLOSE,  // заслінка закр.
            .pulse = 10,            // імпульс мінім. = 100 = 0,5 сек.
            .mode = 11,             // період імпульс. = 3000 = 15 сек.
            .extendMode = 12,       // режим реле = 0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]; 4-импульс
            .Kp = 13,               // пропорц. 1 (Kp/10) = 200
            .Ki = 14,               // ітеграл. 1 (Ki/1000) = 40
        },
        { // Элемент sp_structs[1] (можно оставить пустым для инициализации нулями)
            .spT = 300,             // завдання у грд.Цесія #2 = 300
            .spRH = 16,             // завдання у відсотках #2 = 650
            .alarm = 17,            // аварійне відхилення #2 = 15
            .coolOn = 18,           // знижувач увімкнути #2 = 10
            .coolOff = 19,          // знижувач вимкнути #2 = 5
            .timer = 20,            // лотки вимкн. = 0
            .aeration = 21,         // провітр. робота ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд) = 0
            .auxiliary = 22,        // допоміж. вимкн. = ?
            .state = 23,            // задана програма = 0
            .flapLimit = FLPOPEN,   // заслінка відкр.
            .pulse = 25,            // імпульс максім. = 2000 = 10 сек.
            .mode = 26,             // аварійн. режим = 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
            .extendMode = 27,       // затрим. зволож. = 0
            .Kp = 28,               // пропорц. 2 (Kp/10) = 200
            .Ki = 29,               // ітеграл. 2 (Ki/1000) = 40
        }
    }
};
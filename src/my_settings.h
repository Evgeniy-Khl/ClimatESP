#include "main.h"

// Пин, к которому подключен светодиод (например, D4 на NodeMCU, это GPIO2)
const int ledPin = 2; // GPIO2
// Пин, к которому подключен информационный вывод (DQ) датчика DS18B20
// #define ONE_WIRE_BUS_PIN 0 // используется номер GPIO
// #define MAX_DEVICE 4        // ограничение количества датчиков
// uint8_t numberOfDevices, errDevice[MAX_DEVICE];
// Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
// OneWire oneWire(ONE_WIRE_BUS_PIN);
// Передаем ссылку на объект oneWire в конструктор DallasTemperature
// DallasTemperature sensors(&oneWire);
// Переменная для хранения адреса датчика (если их несколько)
// DeviceAddress sensorAddress;

// Переменные для управления яркостью
int brightness = 0;    // Текущая яркость
int fadeAmount = 5;    // На сколько изменять яркость за один шаг

// Адрес PCF8574. Может быть разным в зависимости от конфигурации A0, A1, A2.
// Стандартные адреса: 0x20-0x27 для PCF8574 и 0x38-0x3F для PCF8574A.
// Уточните адрес вашего модуля. Часто по умолчанию 0x27 или 0x3F.
#define PCF8574_ADDRESS 0x27 // Замените на ваш адрес, если необходимо
long lastMsg = 0, number = 0;
//---------------------------------
char displStr[30];
// Глобальный массив указателей, который будет доступен всем функциям
const char* keyLabel[15];
uint16_t keyColor[15];
bool newDispl = true, newTxt = true;
uint16_t xpos, ypos, txt_height, t_x = 0, t_y = 0; // To store the touch coordinates;
uint8_t displNum=0, seconds=0, pwTriac, dividerValue;
int16_t editValue;
// spT spRH timer alarm coolOn coolOff aeration flapLimit state service pulse mode extendMode Kp Ki Kd
//---------------------------------
Ds ds[2] = {{350,0},{280,0}};
uint16_t set[2] = {385, 305};
int8_t dpv1 = 2;
float flT0=350, dpv0;
//---------------------------------
GrafDispl grafDispl[2] = {
    { 80,80,80, 0, 0},    // Инициализация grafDispl[0]
    {240,80,80, 0, 0},    // Инициализация grafDispl[1]
};

//************************************************************************************************** */
#define FLPCLOSE 0
#define FLPOPEN 255
// РЕКОМЕНДУЕМЫЙ СПОСОБ:
SpUnion settings = {
    .sp_structs = { // Явно говорим, что инициализируем поле sp_structs
        { // Элемент sp_structs[0]
            .spT = 350,
            .spRH = 0,
            .alarm = 10,
            .coolOn = 5,
            .coolOff = 2,
            .timer = 60,
            .aeration = 10,
            .state = 0,
            .flapLimit = FLPCLOSE,
            .service = 10,
            .pulse = 100,
            .mode = 0,
            .extendMode = 0,
            .Kp = 200,
            .Ki = 100,
            .Kd = 1
        },
        { // Элемент sp_structs[1] (можно оставить пустым для инициализации нулями)
            .spT = 300,
            .spRH = 650,
            .alarm = 15,
            .coolOn = 5,
            .coolOff = 2,
            .timer = 0,
            .aeration = 0,
            .state = 0,
            .flapLimit = FLPOPEN,
            .service = 5,
            .pulse = 2000,
            .mode = 0,
            .extendMode = 3000,
            .Kp = 200,
            .Ki = 100,
            .Kd = 1
            // Остальные поля sp_structs[1] будут равны 0
        }
    }
};
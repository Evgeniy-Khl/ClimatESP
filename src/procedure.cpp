#include "main.h"
#include "procedure.h"


void PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd) {
    pid->Kp = (float)Kp/10;
    pid->Ki = (float)Ki/1000;
    pid->Kd = Kd;
}

uint8_t UpdatePID(PIDController *pid, uint8_t cn){
 int16_t error, derivative;
  // Вычисление ошибки
  error = set[cn] - ds[cn].pvT;
  // Пропорциональная составляющая
  pid[cn].pPart = (float)error * pid[cn].Kp;
  // Интегральная составляющая
  pid[cn].iPart += (float)error * pid[cn].Ki;// * dt;
  // Дифференциальная составляющая
  derivative = (error - pid[cn].prev_error);// / dt;
  pid[cn].dPart = pid[cn].Kd * derivative;
  // Сохраняем текущую ошибку для следующего вызова
  pid[cn].prev_error = error;
  // Суммарное управляющее воздействие
  pid[cn].output = pid[cn].pPart + pid[cn].iPart + pid[cn].dPart;
  // Ограничение выходного значения и антивиндовинг
  if (pid[cn].output > 100) pid[cn].output = 110;
  else if (pid[cn].output < 0) pid[cn].output = 0;
  if (pid[cn].pPart >= 100) pid[cn].iPart = 0; // Сброс интеграла
  else if (pid[cn].pPart <= -50) pid[cn].iPart = 0; // Сброс интеграла

  error = pid[cn].output;
  return (uint8_t)error;
}

// Функция для печати текущих значений структуры в Serial порт
void printConfig() {
    Serial.println("--------------------");
    for (int i = 0; i < 2; i++) {
        Serial.printf("Элемент sp[%d]:\n", i);
        Serial.printf("  spT: %d\n", sp[i].spT);
        Serial.printf("  spRH: %d\n", sp[i].spRH);
        Serial.printf("  timer: %d\n", sp[i].timer);
        Serial.printf("  alarm: %d\n", sp[i].alarm);
        Serial.printf("  coolOn: %d\n", sp[i].coolOn);
        Serial.printf("  coolOff: %d\n", sp[i].coolOff);
        Serial.printf("  aeration: %d\n", sp[i].aeration);
        Serial.printf("  flapLimit: %d\n", sp[i].flapLimit);
        Serial.printf("  state: %d\n", sp[i].state);
        Serial.printf("  service: %d\n", sp[i].service);
        Serial.printf("  pulse: %d\n", sp[i].pulse);
        Serial.printf("  mode: %d\n", sp[i].mode);
        Serial.printf("  extendMode: %d\n", sp[i].extendMode);
        Serial.printf("  Kp: %d\n", sp[i].Kp);
        Serial.printf("  Ki: %d\n", sp[i].Ki);
        Serial.printf("  Kd: %d\n", sp[i].Kd);
    }
    Serial.println("--------------------");
}

// Функция сохранения конфигурации в JSON файл
void saveConfig() {
    Serial.println("Сохранение конфигурации...");

    // Создаем JSON документ. Размер 512 байт более чем достаточен.
    StaticJsonDocument<1024> doc;

    // Создаем корневой JSON массив
    JsonArray jsonArray = doc.to<JsonArray>();

    // Проходим по массиву структур и добавляем данные в JSON
    for (int i = 0; i < 2; i++) {
        JsonObject obj = jsonArray.createNestedObject();
        obj["spT"] = sp[i].spT;
        obj["spRH"] = sp[i].spRH;
        obj["timer"] = sp[i].timer;
        obj["alarm"] = sp[i].alarm;
        obj["coolOn"] = sp[i].coolOn;
        obj["coolOff"] = sp[i].coolOff;
        obj["aeration"] = sp[i].aeration;
        obj["flapLimit"] = sp[i].flapLimit;
        obj["state"] = sp[i].state;
        obj["service"] = sp[i].service;
        obj["pulse"] = sp[i].pulse;
        obj["mode"] = sp[i].mode;
        obj["extendMode"] = sp[i].extendMode;
        obj["Kp"] = sp[i].Kp;
        obj["Ki"] = sp[i].Ki;
        obj["Kd"] = sp[i].Kd;
    }

    // Открываем файл для записи
    File configFile = SPIFFS.open("/setpoint.json", "w");
    if (!configFile) {
        Serial.println("Не удалось открыть файл для записи");
        return;
    }

    // Сериализуем JSON в файл
    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Ошибка записи в файл");
    } else {
        Serial.println("Конфигурация успешно сохранена.");
    }
    
    configFile.close();
}

// Функция загрузки конфигурации из JSON файла
bool loadConfig() {
    Serial.println("Загрузка конфигурации...");

    // Открываем файл для чтения
    File configFile = SPIFFS.open("/setpoint.json", "r");
    if (!configFile) {
        Serial.println("Не удалось открыть файл для чтения. Используются значения по умолчанию.");
        return false;
    }

    // Создаем JSON документ для десериализации
    StaticJsonDocument<1024> doc;

    // Десериализуем JSON из файла
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        Serial.print("Ошибка десериализации JSON: ");
        Serial.println(error.c_str());
        configFile.close();
        return false;
    }
// Закрываем файл после чтения
    configFile.close();

    // Получаем корневой JSON массив
    JsonArray jsonArray = doc.as<JsonArray>();

// spT spRH timer alarm coolOn coolOff aeration flapLimit state service pulse mode extendMode Kp Ki Kd
    // Проходим по JSON массиву и заполняем структуру
    int i = 0;
    for (JsonObject obj : jsonArray) {
        if (i < 2) {
            sp[i].spT = obj["spT"];
            sp[i].spRH = obj["spRH"];
            sp[i].timer = obj["timer"];
            sp[i].alarm = obj["alarm"];
            sp[i].coolOn = obj["coolOn"];
            sp[i].coolOff = obj["coolOff"];
            sp[i].aeration = obj["aeration"];
            sp[i].flapLimit = obj["flapLimit"];
            sp[i].state = obj["state"];
            sp[i].service = obj["service"];
            sp[i].pulse = obj["pulse"];
            sp[i].mode = obj["mode"];
            sp[i].extendMode = obj["extendMode"];
            sp[i].Kp = obj["Kp"];
            sp[i].Ki = obj["Ki"];
            sp[i].Kd = obj["Kd"];
            i++;
        }
    }
    Serial.println("Конфигурация успешно загружена.");
    return true;
}

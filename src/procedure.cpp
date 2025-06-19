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
  error = settings.sp_structs[cn].spT - ds[cn].pvT;
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
        Serial.printf("Элемент settings.sp_structs[%d]:\n", i);
        Serial.printf("  spT: %d\n", settings.sp_structs[i].spT);
        Serial.printf("  spRH: %d\n", settings.sp_structs[i].spRH);
        Serial.printf("  alarm: %d\n", settings.sp_structs[i].alarm);
        Serial.printf("  coolOn: %d\n", settings.sp_structs[i].coolOn);
        Serial.printf("  coolOff: %d\n", settings.sp_structs[i].coolOff);
        Serial.printf("  timer: %d\n", settings.sp_structs[i].timer);
        Serial.printf("  aeration: %d\n", settings.sp_structs[i].aeration);
        Serial.printf("  flapLimit: %d\n", settings.sp_structs[i].flapLimit);
        Serial.printf("  state: %d\n", settings.sp_structs[i].state);
        Serial.printf("  service: %d\n", settings.sp_structs[i].service);
        Serial.printf("  pulse: %d\n", settings.sp_structs[i].pulse);
        Serial.printf("  mode: %d\n", settings.sp_structs[i].mode);
        Serial.printf("  extendMode: %d\n", settings.sp_structs[i].extendMode);
        Serial.printf("  Kp: %d\n", settings.sp_structs[i].Kp);
        Serial.printf("  Ki: %d\n", settings.sp_structs[i].Ki);
        Serial.printf("  Kd: %d\n", settings.sp_structs[i].Kd);
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
        obj["spT"] = settings.sp_structs[i].spT;
        obj["spRH"] = settings.sp_structs[i].spRH;
        obj["alarm"] = settings.sp_structs[i].alarm;
        obj["coolOn"] = settings.sp_structs[i].coolOn;
        obj["coolOff"] = settings.sp_structs[i].coolOff;
        obj["timer"] = settings.sp_structs[i].timer;
        obj["aeration"] = settings.sp_structs[i].aeration;
        obj["flapLimit"] = settings.sp_structs[i].flapLimit;
        obj["state"] = settings.sp_structs[i].state;
        obj["service"] = settings.sp_structs[i].service;
        obj["pulse"] = settings.sp_structs[i].pulse;
        obj["mode"] = settings.sp_structs[i].mode;
        obj["extendMode"] = settings.sp_structs[i].extendMode;
        obj["Kp"] = settings.sp_structs[i].Kp;
        obj["Ki"] = settings.sp_structs[i].Ki;
        obj["Kd"] = settings.sp_structs[i].Kd;
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
            settings.sp_structs[i].spT = obj["spT"];
            settings.sp_structs[i].spRH = obj["spRH"];
            settings.sp_structs[i].alarm = obj["alarm"];
            settings.sp_structs[i].coolOn = obj["coolOn"];
            settings.sp_structs[i].coolOff = obj["coolOff"];
            settings.sp_structs[i].timer = obj["timer"];
            settings.sp_structs[i].aeration = obj["aeration"];
            settings.sp_structs[i].flapLimit = obj["flapLimit"];
            settings.sp_structs[i].state = obj["state"];
            settings.sp_structs[i].service = obj["service"];
            settings.sp_structs[i].pulse = obj["pulse"];
            settings.sp_structs[i].mode = obj["mode"];
            settings.sp_structs[i].extendMode = obj["extendMode"];
            settings.sp_structs[i].Kp = obj["Kp"];
            settings.sp_structs[i].Ki = obj["Ki"];
            settings.sp_structs[i].Kd = obj["Kd"];
            i++;
        }
    }
    Serial.println("Конфигурация успешно загружена.");
    return true;
}

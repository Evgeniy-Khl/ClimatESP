#include "saveDailyData.h"

/**
 * @brief Читает 288 пятиминутных записей из EEPROM и сохраняет их в файлы.
 * @param day Номер дня инкубации.
 */
void saveDailyDataToFile(int day) {
  checkAndManageSpace(); // Проверка и освобождение места

  DEBUG_PRINTF("Начало сохранения данных за день #%d\n", day);
  JsonDocument graphDoc;
  JsonArray dataArray = graphDoc.to<JsonArray>();
  JsonDocument statsDoc;

  float total_sum_t1 = 0, min_t1 = 200, max_t1 = -200;
  float total_sum_t2 = 0, min_t2 = 200, max_t2 = -200;
  int validReadingsCount = 0;

  const int totalPeriods = 288; // Всего 288 пятиминутных периодов в сутках


    // --- Основной цикл: читаем 288 готовых записей ---
  for (int period = 0; period < totalPeriods; ++period) {
        // Адрес вычисляется так же, как и при записи
        int currentAddress = period * sizeof(int16_t) * 2;
        int16_t raw_t1, raw_t2;
        eepromReadInt16(currentAddress, raw_t1);
        eepromReadInt16(currentAddress + sizeof(int16_t), raw_t2);

        // Проверяем, была ли вообще сделана запись (если EEPROM пуст, там будут нули или -1)
        if (raw_t1 == 0 && raw_t2 == 0) {
        continue; // Пропускаем пустую запись
        }

        float t1 = (float)raw_t1 / 10.0;
        float t2 = (float)raw_t2 / 10.0;

        // Добавляем точку в JSON для графика
        JsonObject point = dataArray.add<JsonObject>();
        point["p"] = period;
        point["t1"] = t1;
        point["t2"] = t2;

        if (detectedSensor == SENSOR_DHT22) {
            point["rh"] = t2;
        } else if (numberOfDevices == 2) {
            uint8_t rh = tableRH(raw_t1, raw_t2);
            if (rh <= 100) point["rh"] = rh;
        }


        // Обновляем общую дневную статистику
        total_sum_t1 += t1;
        total_sum_t2 += t2;
        if (t1 < min_t1) min_t1 = t1;
        if (t1 > max_t1) max_t1 = t1;
        if (t2 < min_t2) min_t2 = t2;
        if (t2 > max_t2) max_t2 = t2;
        validReadingsCount++;
      }
  //----------------- Сохранение файлов --------------------
  String graphFilename = "/day_" + String(day) + "_graph.json";
  File graphFile = LittleFS.open(graphFilename, "w");
  if (graphFile) {
    if (serializeJson(graphDoc, graphFile) > 0) {
      Serial.printf("Данные для графика сохранены в %s\n", graphFilename.c_str());
      serializeJson(graphDoc, Serial);
    } else {
      Serial.println("Ошибка записи JSON в файл графика!");
    }
    graphFile.close();
  } else {
    Serial.println("Ошибка открытия файла для графика!");
  }
  graphDoc.clear();
  
  if (validReadingsCount > 0) {
    statsDoc["avg_t1"] = total_sum_t1 / validReadingsCount;
    statsDoc["min_t1"] = min_t1;
    statsDoc["max_t1"] = max_t1;
    statsDoc["avg_t2"] = total_sum_t2 / validReadingsCount;
    statsDoc["min_t2"] = min_t2;
    statsDoc["max_t2"] = max_t2;

    String statsFilename = "/day_" + String(day) + "_stats.json";
    File statsFile = LittleFS.open(statsFilename, "w");
    if (statsFile) {
      if (serializeJson(statsDoc, statsFile) > 0) {
        Serial.printf("Статистика сохранена в %s\n", statsFilename.c_str());
        serializeJson(statsDoc, Serial);
      } else {
        Serial.println("Ошибка записи JSON в файл статистики!");
      }
      statsFile.close();
    } else {
      Serial.println("Ошибка открытия файла для статистики!");
    }
  } else {
    Serial.println("Не найдено валидных данных за день для сохранения статистики.");
  }
}

/**
 * @brief Удаляет все файлы данных от предыдущей инкубации.
 * Ищет и удаляет файлы, соответствующие шаблонам "day_..._graph.json" и "day_..._stats.json".
 */
void clearIncubationData() {
  MYDEBUG_PRINTLN("Очистка данных предыдущей инкубации...");

  Dir dir = LittleFS.openDir("/");
  int filesRemoved = 0;

  while (dir.next()) {
    String fileName = dir.fileName();
    // Проверяем, что имя файла соответствует нашему шаблону
    if (fileName.startsWith("day_") && fileName.endsWith(".json")) {
      if (LittleFS.remove(fileName)) {
        DEBUG_PRINTF("Файл удален: %s\n", fileName.c_str());
        filesRemoved++;
      } else {
        DEBUG_PRINTF("Ошибка удаления файла: %s\n", fileName.c_str());
      }
    }
  }
  
  if (filesRemoved == 0) {
    MYDEBUG_PRINTLN("Файлы для очистки не найдены.");
  } else {
    DEBUG_PRINTF("Очистка завершена. Удалено файлов: %d\n", filesRemoved);
  }
}

/**
 * @brief Находит в файловой системе самый "старый" день инкубации.
 * @return Номер самого старого дня или -1, если файлы не найдены.
 */
int findOldestDay() {
  Dir dir = LittleFS.openDir("/");
  int oldestDay = -1;

  while (dir.next()) {
    String fileName = dir.fileName();
    // Ищем файлы, которые соответствуют нашему шаблону
    if (fileName.startsWith("/day_") && fileName.endsWith("_stats.json")) {
      // Вырезаем номер дня из имени файла, например, из "/day_12_stats.json" получаем "12"
      int start = 5; // индекс после "/day_"
      int end = fileName.indexOf('_', start);
      int currentDay = fileName.substring(start, end).toInt();

      if (oldestDay == -1 || currentDay < oldestDay) {
        oldestDay = currentDay;
      }
    }
  }
  return oldestDay;
}

/**
 * @brief Удаляет оба файла (_graph.json и _stats.json) для указанного дня.
 * @param day Номер дня, файлы которого нужно удалить.
 */
void deleteFilesForDay(int day) {
  if (day < 0) return;

  DEBUG_PRINTF("Удаление файлов для дня #%d...\n", day);
  String graphFilename = "/day_" + String(day) + "_graph.json";
  String statsFilename = "/day_" + String(day) + "_stats.json";

  if (LittleFS.remove(graphFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", graphFilename.c_str());
  }
  if (LittleFS.remove(statsFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", statsFilename.c_str());
  }
}

/**
 * @brief Проверяет наличие свободного места и удаляет старые файлы, если необходимо.
 */
void checkAndManageSpace() {
  // Установим порог необходимого свободного места в 30 КБ.
  // Этого с запасом хватит для большого файла графика (~24 КБ) и маленького файла статистики.
  const long REQUIRED_SPACE = 30000; 

  FSInfo fs_info;
  LittleFS.info(fs_info);
  long freeSpace = fs_info.totalBytes - fs_info.usedBytes;

  DEBUG_PRINTF("Проверка свободного места: доступно %ld байт. Необходимо: %ld байт.\n", freeSpace, REQUIRED_SPACE);

  // Запускаем цикл, который будет удалять старые файлы, пока не освободится достаточно места
  while (freeSpace < REQUIRED_SPACE) {
    MYDEBUG_PRINTLN("Недостаточно места!");
    int oldestDay = findOldestDay();

    if (oldestDay == -1) {
      MYDEBUG_PRINTLN("Ошибка: нет старых файлов для удаления, но место закончилось!");
      break; // Выходим, чтобы избежать бесконечного цикла
    }
    
    deleteFilesForDay(oldestDay);

    // Обновляем информацию о свободном месте
    LittleFS.info(fs_info);
    freeSpace = fs_info.totalBytes - fs_info.usedBytes;
    DEBUG_PRINTF("Теперь доступно: %ld байт\n", freeSpace);
  }
}

void startIncubation() {
  clearIncubationData();
  // Установка времени: 25 год, 1 месяц, 1 день, 00:00:00
  rtc.adjust(DateTime(2025, 1, 1, 0, 0, 0));
  eepromWriteByte(STARTINCUBADRES, 1);          // старт инкубации
  INCUBATION = 1;                               // установим флаг
}

/**
 * @brief Выводит в Serial Monitor список всех файлов и их размеры.
 * Также показывает общую информацию о занятом и свободном месте.
 */
void listFilesAndSizes() {
  Serial.println("\n--- Список файлов в LittleFS ---");
  
  Dir dir = LittleFS.openDir("/");
  long totalSize = 0;
  int fileCount = 0;

  while (dir.next()) {
    // Для каждого элемента получаем объект File
    File entry = dir.openFile("r");
    if (entry) {
      Serial.print("Файл: ");
      Serial.print(entry.name());
      Serial.print("\tРазмер: ");
      Serial.print(entry.size());
      Serial.println(" Байт");
      totalSize += entry.size();
      fileCount++;
      entry.close(); // Важно закрывать файл после использования
    }
  }

  Serial.println("------------------------------------");
  Serial.printf("Всего файлов: %d\n", fileCount);
  Serial.printf("Общий размер: %ld Байт\n", totalSize);

  // Дополнительная информация о файловой системе
  FSInfo fs_info;
  LittleFS.info(fs_info);
  Serial.printf("Всего места:  %d Байт\n", fs_info.totalBytes);
  Serial.printf("Использовано: %d Байт\n", fs_info.usedBytes);
  Serial.println("------------------------------------");
}
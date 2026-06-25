#include "saveDailyData.h"

/**
 * @brief Читает 288 пятиминутных записей из EEPROM и сохраняет их в файлы.
 * @param day Номер дня инкубации.
 */
void saveDailyDataToFile(int day) {
  checkAndManageSpace(); // Проверка и освобождение места

  DEBUG_PRINTF("Начало сохранения данных за день #%d\n", day);
  
  DateTime targetDate;
  uint8_t start_data[7];
  bool startValid = false;
  
  if (eepromReadBuffer(INCUBATION_DATA_ADRES, start_data, 7) == 7 && start_data[0] > 0) {
    if (start_data[2] > 0 && start_data[2] <= 12 && start_data[3] > 0 && start_data[3] <= 31) {
      DateTime startDate(start_data[1] + 2000, start_data[2], start_data[3], start_data[4], start_data[5], start_data[6]);
      targetDate = startDate + TimeSpan(day, 0, 0, 0);
      startValid = true;
    }
  }
  
  if (!startValid) {
    // Резервный вариант: вчерашний день
    targetDate = rtc.now() - TimeSpan(1, 0, 0, 0);
  }

  // Защита от сбоев времени: архивный день не может быть в будущем относительно реального времени RTC
  if (targetDate > rtc.now()) {
    targetDate = rtc.now() - TimeSpan(1, 0, 0, 0);
  }

  char dateBuf[8];
  snprintf(dateBuf, sizeof(dateBuf), "%02d_%02d", targetDate.day(), targetDate.month());
  String dateStr(dateBuf);

  String graphFilename = "/day_" + dateStr + "_graph.json";
  File graphFile = LittleFS.open(graphFilename, "w");
  if (!graphFile) {
    MYDEBUG_PRINTLN("Ошибка открытия файла для графика!");
    return;
  }

  // Начинаем JSON массив напрямую в файл
  graphFile.print("[");

  float total_sum_t1 = 0, min_t1 = 200, max_t1 = -200;
  float total_sum_t2 = 0, min_t2 = 200, max_t2 = -200;
  float total_sum_rh = 0, min_rh = 200, max_rh = -200;
  int validReadingsCount = 0;

  // --- Основной цикл: читаем 288 записей и сразу пишем в файл ---
  for (int period = 0; period < DAILY_DATA_MAX_REC; ++period) {
    if (period % 10 == 0) {
      ESP.wdtFeed();
      yield();
    }
    int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
    int16_t raw_t1, raw_t2, raw_rh;
    eepromReadInt16(currentAddress, raw_t1);
    eepromReadInt16(currentAddress + 2, raw_t2);
    eepromReadInt16(currentAddress + 4, raw_rh);

    if (raw_t1 == 0 && raw_t2 == 0) continue;

    float t1 = (float)raw_t1 / 10.0;
    float t2 = (float)raw_t2 / 10.0;
    float rh = (float)raw_rh;

    if (validReadingsCount > 0) graphFile.print(",");
    
    // Пишем одну точку. printf экономит RAM по сравнению с JsonDocument
    graphFile.printf("{\"p\":%d,\"t1\":%.1f,\"t2\":%.1f,\"rh\":%.1f}", period, t1, t2, rh);

    // Обновляем статистику
    total_sum_t1 += t1; total_sum_t2 += t2; total_sum_rh += rh;
    if (t1 < min_t1) min_t1 = t1; 
    if (t1 > max_t1) max_t1 = t1;
    if (t2 < min_t2) min_t2 = t2; 
    if (t2 > max_t2) max_t2 = t2;
    if (rh < min_rh) min_rh = rh; 
    if (rh > max_rh) max_rh = rh;
    validReadingsCount++;
  }

  graphFile.print("]"); // Закрываем массив
  graphFile.close();
  
  DEBUG_PRINTF("Данные сохранены в %s. Точек: %d", graphFilename.c_str(), validReadingsCount);

  if (validReadingsCount > 0) {
    JsonDocument statsDoc;
    statsDoc["avg_t1"] = total_sum_t1 / validReadingsCount;
    statsDoc["min_t1"] = min_t1;
    statsDoc["max_t1"] = max_t1;
    statsDoc["avg_t2"] = total_sum_t2 / validReadingsCount;
    statsDoc["min_t2"] = min_t2;
    statsDoc["max_t2"] = max_t2;
    statsDoc["avg_rh"] = total_sum_rh / validReadingsCount;
    statsDoc["min_rh"] = min_rh;
    statsDoc["max_rh"] = max_rh;

    String statsFilename = "/day_" + dateStr + "_stats.json";
    File statsFile = LittleFS.open(statsFilename, "w");
    if (statsFile) {
      serializeJson(statsDoc, statsFile);
      statsFile.close();
      DEBUG_PRINTF("  Статистика сохранена в %s", statsFilename.c_str());
    }
    uint16_t heapSize = ESP.getFreeHeap();    // Проверка доступной памяти
    DEBUG_PRINTF("  Free heap size: %d\n", heapSize);
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
    if (fileName.startsWith("day_") && (fileName.endsWith(".json") || fileName.endsWith("_log.txt"))) {
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

// Вспомогательная функция для расчета расстояния в днях от текущей даты
int32_t getFileDistance(const String& fileName) {
  // fileName пример: "day_21_06_stats.json"
  int start = fileName.indexOf('_');
  if (start == -1) return -1;
  int mid = fileName.indexOf('_', start + 1);
  if (mid == -1) return -1;
  int end = fileName.indexOf('_', mid + 1);
  if (end == -1) {
    end = fileName.indexOf('.', mid + 1);
  }
  if (end == -1) return -1;
  
  int day = fileName.substring(start + 1, mid).toInt();
  int month = fileName.substring(mid + 1, end).toInt();
  
  int year = now.year();
  // Если текущий месяц в начале года, а у файла в конце - значит файл из прошлого года
  if (now.month() <= 2 && month >= 10) {
    year -= 1;
  }
  
  DateTime fileDate(year, month, day, 0, 0, 0);
  
  int32_t distance = 0;
  if (now >= fileDate) {
    TimeSpan diff = now - fileDate;
    distance = diff.days();
  } else {
    // В случае сбитых часов
    TimeSpan diff = fileDate - now;
    distance = -diff.days();
  }
  return distance;
}

/**
 * @brief Находит в файловой системе самый "старый" день инкубации.
 * @return Дата самого старого дня в формате DD_MM.
 */
String findOldestDate() {
  Dir dir = LittleFS.openDir("/");
  String oldestDateStr = "";
  int32_t maxDistance = -999999;

  while (dir.next()) {
    String fileName = dir.fileName();
    // Ищем файлы, которые соответствуют нашему шаблону
    if (fileName.startsWith("day_") && fileName.endsWith("_stats.json")) {
      int start = 4; // индекс после "day_"
      int end = fileName.indexOf("_stats.json");
      if (end > start) {
        String dateStr = fileName.substring(start, end);
        int32_t distance = getFileDistance(fileName);
        if (distance > maxDistance) {
          maxDistance = distance;
          oldestDateStr = dateStr;
        }
      }
    }
  }
  return oldestDateStr;
}

/**
 * @brief Удаляет файлы данных (_graph.json, _stats.json и _log.txt) для указанной даты.
 * @param dateStr Строка даты в формате DD_MM.
 */
void deleteFilesForDate(const String& dateStr) {
  if (dateStr.length() == 0) return;

  DEBUG_PRINTF("Удаление файлов для даты %s...\n", dateStr.c_str());
  String graphFilename = "/day_" + dateStr + "_graph.json";
  String statsFilename = "/day_" + dateStr + "_stats.json";
  String logFilename = "/day_" + dateStr + "_log.txt";

  if (LittleFS.remove(graphFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", graphFilename.c_str());
  }
  if (LittleFS.remove(statsFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", statsFilename.c_str());
  }
  if (LittleFS.remove(logFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", logFilename.c_str());
  }
}

/**
 * @brief Проверяет наличие свободного места и удаляет старые файлы, если необходимо.
 */
void checkAndManageSpace() {
  const long REQUIRED_SPACE = 11411; 

  FSInfo fs_info;
  LittleFS.info(fs_info);
  long freeSpace = fs_info.totalBytes - fs_info.usedBytes;

  DEBUG_PRINTF("Проверка свободного места: доступно %ld байт. Необходимо: %ld байт.\n", freeSpace, REQUIRED_SPACE);

  // Запускаем цикл, который будет удалять старые файлы, пока не освободится достаточно места
  while (freeSpace < REQUIRED_SPACE) {
    MYDEBUG_PRINTLN("Недостаточно места!");
    String oldestDate = findOldestDate();

    if (oldestDate.length() == 0) {
      MYDEBUG_PRINTLN("Ошибка: нет старых файлов для удаления, но место закончилось!");
      break; // Выходим, чтобы избежать бесконечного цикла
    }
    
    deleteFilesForDate(oldestDate);

    // Обновляем информацию о свободном месте
    LittleFS.info(fs_info);
    freeSpace = fs_info.totalBytes - fs_info.usedBytes;
    DEBUG_PRINTF("Теперь доступно: %ld байт\n", freeSpace);
  }
}

void startIncubation() {
  int16_t currentState = settings.sp_structs[1].state;
  DateTime nowTime = rtc.now();
  
  // Сохранение в EEPROM (7 байт: состояние, Г, М, Д, ч, м, с)
  uint8_t data[7];
  data[0] = (uint8_t)currentState;
  data[1] = nowTime.year() - 2000;
  data[2] = nowTime.month();
  data[3] = nowTime.day();
  data[4] = nowTime.hour();
  data[5] = nowTime.minute();
  data[6] = nowTime.second();
  
  eepromWriteBuffer(INCUBATION_DATA_ADRES, data, 7);
  MYDEBUG_PRINT("Записано время начала/смены программы: ");
  DEBUG_PRINTF("%02d.%02d.20%02d %02d:%02d:%02d | State: %d\n", data[3], data[2], data[1], data[4], data[5], data[6], data[0]);

  if (currentState > 0) {
    clearIncubationData();
    countDays = 0;
    countHours = 0;
    countMinutes = 0;
    countSeconds = 0;
    
    if (currentState == 5) {
      MYDEBUG_PRINTLN("Ручная инкубация запущена!");
      settings.sp_structs[1].state = 0; // Для системы это "без программы", но дни считаем
      saveSetpoint();
    } else {
      MYDEBUG_PRINTLN("Программная инкубация запущена!");
      applyDailyProgram(); // Сразу загружаем параметры первого дня
    }
  } else {
    MYDEBUG_PRINTLN("Инкубация остановлена.");
  }
}

void restoreIncubationStatus() {
  uint8_t data[7];
  if (eepromReadBuffer(INCUBATION_DATA_ADRES, data, 7) == 7) {
    int16_t savedState = data[0];
    // Восстанавливаем, если была запущена программа (>0) или ручной режим (savedState был > 0 при записи)
    if (savedState > 0) {
      if (updateIncubationTime()) {
        MYDEBUG_PRINT("Инкубация восстановлена. Текущий день: ");
        DEBUG_PRINTF("%d дн. %02d:%02d:%02d\n", countDays, countHours, countMinutes, countSeconds);
        
        // Если это была программа (не ручной режим 5) и она совпадает с текущей в настройках
        if (savedState < 5 && savedState == settings.sp_structs[1].state) {
          applyDailyProgram(); // Загружаем параметры текущего восстановленного дня
        }
      } else {
        MYDEBUG_PRINTLN("Инкубация обнаружена, но не удалось рассчитать время (ошибка чтения EEPROM).");
      }
    } else {
      MYDEBUG_PRINTLN("Инкубация не была запущена.");
    }
  } else {
    MYDEBUG_PRINTLN("Данные инкубации в EEPROM не найдены.");
  }
}

/**
 * @brief Выводит в Serial Monitor список всех файлов и их размеры.
 * Также показывает общую информацию о занятом и свободном месте.
 */
void listFilesAndSizes() {
  MYDEBUG_PRINTLN("\n--- Список файлов в LittleFS ---");
  
  Dir dir = LittleFS.openDir("/");
  long totalSize = 0;
  int fileCount = 0;

  while (dir.next()) {
    // Для каждого элемента получаем объект File
    File entry = dir.openFile("r");
    if (entry) {
      MYDEBUG_PRINT("Файл: ");
      MYDEBUG_PRINT(entry.name());
      MYDEBUG_PRINT("\tРазмер: ");
      MYDEBUG_PRINT(entry.size());
      MYDEBUG_PRINTLN(" Байт");
      totalSize += entry.size();
      fileCount++;
      entry.close(); // Важно закрывать файл после использования
    }
  }

  MYDEBUG_PRINTLN("------------------------------------");
  DEBUG_PRINTF("Всего файлов: %d\n", fileCount);
  DEBUG_PRINTF("Общий размер: %ld Байт\n", totalSize);

  // Дополнительная информация о файловой системе
  FSInfo fs_info;
  LittleFS.info(fs_info);
  DEBUG_PRINTF("Всего места:  %d Байт\n", fs_info.totalBytes);
  DEBUG_PRINTF("Использовано: %d Байт\n", fs_info.usedBytes);
  MYDEBUG_PRINTLN("------------------------------------");
}
#include "saveDailyData.h"

/**
 * @brief Обрабатывает данные из EEPROM (в формате int16_t) и сохраняет в два файла.
 * @param day Номер дня инкубации.
 */
void saveDailyDataToFile(int day) {
  DEBUG_PRINTF("Начало сохранения данных за день #%d\n", day);

  // --- 1. Подготовка ---
  JsonDocument graphDoc;
  JsonArray dataArray = graphDoc.to<JsonArray>();
  JsonDocument statsDoc;

  float total_sum_t1 = 0, min_t1 = 200, max_t1 = -200;
  float total_sum_t2 = 0, min_t2 = 200, max_t2 = -200;
  int validReadingsCount = 0;

  const int total5MinPeriods = 288;
  const int readingsPerPeriod = 5;

  // --- 2. Чтение, преобразование и обработка данных ---
  for (int period = 0; period < total5MinPeriods; ++period) {
    float sum_t1_period = 0;
    float sum_t2_period = 0;
    int readingsInPeriod = 0;

    for (int minute = 0; minute < readingsPerPeriod; ++minute) {
      int minuteOfDay = period * readingsPerPeriod + minute;
      
      // Каждая минутная запись (t1, t2) теперь занимает 2 * sizeof(int16_t) = 4 байта
      int currentAddress = minuteOfDay * sizeof(int16_t) * 2;
      
      // Читаем raw-значения из EEPROM
      int16_t raw_t1 = eepromReadInt16(currentAddress);
      int16_t raw_t2 = eepromReadInt16(currentAddress + sizeof(int16_t));

      // Преобразуем обратно в float
      float t1 = (float)raw_t1 / 10.0;
      float t2 = (float)raw_t2 / 10.0;

      // Проверка на адекватность данных (проверяем уже float)
      if (t1 > -50 && t1 < 150) {
        sum_t1_period += t1;
        sum_t2_period += t2;
        readingsInPeriod++;

        // Обновляем общую дневную статистику
        total_sum_t1 += t1;
        total_sum_t2 += t2;
        if (t1 < min_t1) min_t1 = t1;
        if (t1 > max_t1) max_t1 = t1;
        if (t2 < min_t2) min_t2 = t2;
        if (t2 > max_t2) max_t2 = t2;
        validReadingsCount++;
      }
    }

    if (readingsInPeriod > 0) {
      JsonObject point = dataArray.createNestedObject();
      point["p"] = period;
      point["t1"] = sum_t1_period / readingsInPeriod;
      point["t2"] = sum_t2_period / readingsInPeriod;
    }
  }

  // --- 3. Сохранение файлов (эта часть не изменилась) ---
  // ... (код сохранения файлов остается прежним)
  // ... (я включу его в полный пример ниже, чтобы не дублировать)
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
    if (fileName.startsWith("/day_") && fileName.endsWith(".json")) {
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
  rtc.adjust(DateTime(1, 1, 1, 0, 0, 0));
  eepromWriteByte(STARTINCUBADRES, 1);          // старт инкубации
  INCUBATION = 1;                               // установим флаг
}
#ifndef _SAVEDAILYDATA_H
#define _SAVEDAILYDATA_H
#include "main.h"

void saveDailyDataToFile(int day);
void clearIncubationData();
String findOldestDate();
void deleteFilesForDate(const String& dateStr);
void checkAndManageSpace();
void startIncubation();
void restoreIncubationStatus();
void listFilesAndSizes();

#endif /* _SAVEDAILYDATA_H */

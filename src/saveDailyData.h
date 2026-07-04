#ifndef _SAVEDAILYDATA_H
#define _SAVEDAILYDATA_H
#include "main.h"

struct ArchiveDay {
    int day;
    int month;
};

void saveDailyDataToFile(int day);
void clearIncubationData();
ArchiveDay findOldestDay();
void deleteFilesForDay(int day, int month);
void checkAndManageSpace(long requiredSpace = 11411);
void startIncubation();
void restoreIncubationStatus();
void listFilesAndSizes();

#endif /* _SAVEDAILYDATA_H */

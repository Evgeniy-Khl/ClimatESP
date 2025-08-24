#ifndef _SAVEDAILYDATA_H
#define _SAVEDAILYDATA_H
#include "main.h"

void saveDailyDataToFile(int day);
void clearIncubationData();
int findOldestDay();
void deleteFilesForDay(int day);
void checkAndManageSpace();
void startIncubation();


#endif /* _SAVEDAILYDATA_H */

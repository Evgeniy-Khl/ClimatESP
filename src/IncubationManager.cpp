#include "IncubationManager.h"
#include "main.h"
#include "logger.h"

void IncubationManager::init() {
    logEvent("Менеджер інкубації ініціалізовано.");
}

void IncubationManager::tick() {
    // 1. Расчет ПИД-регуляторов и режима нагрева/увлажнения
    processPID();

    // 2. Управление дополнительным каналом обогрева/увлажнения
    if (ERROR1 == 0 && (settings.sp_structs[1].mode & 2) == 0) { 
        if (ds[0].pvErr >= settings.sp_structs[0].auxiliary) {
            EXTRA2 = PCF_ON;
        } else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) {
            EXTRA2 = PCF_OFF;
        }
    } else {
        EXTRA2 = PCF_OFF;
    }

    // 3. Управление проветриванием и охлаждением
    processVentilation();

    // 4. Управление поворотом лотков
    processRotation();

    // 5. Контроль аварийных ситуаций
    processAlarm();

    // 6. Расчет мощности для вывода в процентах
    pctHeater = constrain(heaterValue, 0, 255);
    pctHeater = map(pctHeater, 0, 255, 0, 100);
    
    pctHimidifier = constrain(humidiValue, 0, 255);
    pctHimidifier = map(pctHimidifier, 0, 255, 0, 100);

    // 7. Автоматическое циклическое логирование статусов ошибок при изменении
    // static byte last_errors_val = 0;
    // if (errorsFlag.value != last_errors_val) {
    //     byte changed = errorsFlag.value ^ last_errors_val;
    //     for (int i = 0; i < 8; i++) {
    //         if (changed & (1 << i)) {
    //             bool isSet = (errorsFlag.value & (1 << i));
    //             const char* errName = "";
    //             switch(i) {
    //                 case 0: errName = "Помилка датчика №1 (відключений/завис)"; break;
    //                 case 1: errName = "Помилка датчика №2 (відключений/завис)"; break;
    //                 case 2: errName = "Відхилення по каналу №1"; break;
    //                 case 3: errName = "Відхилення по каналу №2"; break;
    //                 case 4: errName = "Захист від розносу температури"; break;
    //                 case 5: errName = "Зависання датчика"; break;
    //                 case 6: errName = "Резерв"; break;
    //                 case 7: errName = "Перегрів симістора"; break;
    //             }
    //             if (errName[0] != '\0' && i != 6) { // Пропускаем резервный бит
    //                 logEvent("Статус [%s] -> %s", errName, isSet ? "АКТИВОВАНО" : "СКАСОВАНО");
    //             }
    //         }
    //     }
    //     last_errors_val = errorsFlag.value;
    // }
}

void IncubationManager::processPID() {
    checkModeDevice();
    
    // Задержка регулирования по 2 каналу до прогрева инкубатора
    if ((settings.sp_structs[1].mode & 1) == 1 && ds[0].deviation == 0) {
        humidiValue = TRIACOFF;
    }
}

void IncubationManager::processVentilation() {
    if (AERATION) { // Идет ПРОВЕТРИВАНИЕ!
        EXTRA1 = PCF_ON; 
        pvFlap = settings.sp_structs[1].flapLimit; // полностью открыта
        beeperOn(10);
        if (--pvVenting == 0) {
            pvAeration = settings.sp_structs[0].aeration; 
            AERATION = 0; 
            EXTRA1 = PCF_OFF;
        }
    } else { // Регулировка охлаждения/осушения заслонкой
        uint8_t val = 0;
        if (settings.sp_structs[1].extendMode & 3)      // 1-ОХЛАЖДЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ
            val = RelayNeg(0, settings.sp_structs[0].coolOn, settings.sp_structs[0].coolOff);   // нужно ли охлаждать
        if (settings.sp_structs[1].extendMode & 2) {    // 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ
            val = RelayNeg(1, settings.sp_structs[1].coolOn, settings.sp_structs[1].coolOff);       // нужно ли осушать
        }
        if (val == ON) {
            EXTRA1 = PCF_ON; 
            pvFlap = settings.sp_structs[1].flapLimit;  // полностью открыта
        } else if (val == OFF) {
            EXTRA1 = PCF_OFF; 
            pvFlap = settings.sp_structs[0].state;      // текущее положение
        }
        if (ds[0].pvErr > settings.sp_structs[0].alarm) 
            pvFlap = settings.sp_structs[0].flapLimit;  // полностью закрыта
    }
}

void IncubationManager::processRotation() {
    if (settings.sp_structs[1].timer && TURN) { // Асимметричный режим
        TURNSECOND = ON;
        if (--pvTimer == 0) {
            pvTimer = settings.sp_structs[0].timer; 
            TURN = PCF_OFF; 
            TURNSECOND = OFF;
        }
    } else {
        TURNSECOND = OFF;
    }
}

void IncubationManager::processAlarm() {
    if (numSetup == 0) {
        uint8_t res = alarm();
        switch (settings.sp_structs[0].extendMode) {    // 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ
            case 0: 
                EXTRA3 = !res; 
                break;
            case 1: {
                uint8_t val = RelayNeg(0, settings.sp_structs[0].alarm, settings.sp_structs[0].spT);
                if (val == ON) EXTRA3 = PCF_ON;
                else if (val == OFF) EXTRA3 = PCF_OFF;
                break;
            }
        }
    }
}

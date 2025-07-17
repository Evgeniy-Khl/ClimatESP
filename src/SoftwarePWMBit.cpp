#include "main.h"
#include "SoftwarePWMBit.h" // Подключаем наш заголовочный файл

// Реализация методов класса SoftwarePWMBit
// =================================================================

// Конструктор
SoftwarePWMBit::SoftwarePWMBit(unsigned char* byte, uint8_t bitPosition) {
    targetByte = byte;
    bitMask = 1 << bitPosition;
    dutyCycle = 0;
    periodMillis = 1000;
    lastCycleStartMicros = millis();
}

// Метод для установки скважности
void SoftwarePWMBit::write(int value) {
    dutyCycle = constrain(value, 0, TRIACON);
    dutyCycle = TRIACON - dutyCycle;
}

// Главный метод обновления
bool SoftwarePWMBit::update() {
    unsigned long now = millis();
    bool stateChanged = false;

    if (now - lastCycleStartMicros >= periodMillis) {
        lastCycleStartMicros += periodMillis;
    }

    unsigned long onTime = (periodMillis * dutyCycle) / 255;
    
    bool shouldBeOn = (now - lastCycleStartMicros < onTime);
    bool isCurrentlyOn = (*targetByte & bitMask) != 0;

    if (shouldBeOn != isCurrentlyOn) {
        stateChanged = true;
        if (shouldBeOn) {
            *targetByte |= bitMask;
        } else {
            *targetByte &= ~bitMask;
        }
    }
    
    return stateChanged;
}
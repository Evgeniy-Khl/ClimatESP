#include "InvertedServo.h"

// Переменные для работы прерывания таймера
static volatile uint32_t servoPulseTicks = 7500; // По умолчанию 1500 мкс (1500 * 5 тиков при TIM_DIV16)
static volatile bool servoPinState = true;       // true = HIGH, false = LOW
static int servoPin = -1;

// Обработчик прерывания таймера 1 (выполняется из RAM)
void IRAM_ATTR onTimer1Interrupt() {
  if (servoPinState) {
    // Закончилась пауза, начинается импульс.
    // Из-за инвертирующего транзистора нам нужен HIGH на входе сервы, 
    // поэтому на пине ESP8266 мы выставляем LOW.
    if (servoPin >= 0) {
      digitalWrite(servoPin, LOW);
    }
    servoPinState = false;
    // Настраиваем таймер на время импульса
    timer1_write(servoPulseTicks);
  } else {
    // Закончился импульс, начинается пауза.
    // Нам нужен LOW на входе сервы, поэтому на пине ESP8266 мы выставляем HIGH.
    if (servoPin >= 0) {
      digitalWrite(servoPin, HIGH);
    }
    servoPinState = true;
    // Настраиваем таймер на время паузы (период 20 мс = 100000 тиков при TIM_DIV16)
    timer1_write(100000 - servoPulseTicks);
  }
}

InvertedServo::InvertedServo() {
  _pin = -1;
}

void InvertedServo::attach(int pin) {
  _pin = pin;
  servoPin = pin;
  
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH); // Транзистор открыт, на выходе к серве LOW (состояние паузы)
  servoPinState = true;

  timer1_isr_init();
  timer1_attachInterrupt(onTimer1Interrupt);
  // TIM_DIV16 дает частоту таймера 5 МГц (80 МГц / 16). 1 тик = 0.2 мкс.
  // TIM_SINGLE означает однократный запуск таймера (перезапускаем вручную в ISR).
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(100000); // Начинаем с паузы (20 мс = 100000 тиков)
}

void InvertedServo::writeMicroseconds(int us) {
  if (us < 544) us = 544;
  if (us > 2400) us = 2400;
  
  noInterrupts();
  servoPulseTicks = us * 5; // 1 мкс = 5 тиков при частоте 5 МГц
  interrupts();
}

void InvertedServo::write(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  
  // 100% открытия заслонки соответствует углу в 90 градусов (максимальный диапазон хода)
  int angle = map(percent, 0, 100, 0, 90);
  int us = map(angle, 0, 180, 544, 2400);
  writeMicroseconds(us);
}

#include "tftArcFill.h"
#include "procedure.h"

void PID_Init(PIDController *pid) { // , uint16_t Kp, uint16_t Ki, uint16_t Kd
    pid->Kp = 10;//(float)Kp/10;
    pid->Ki = 50;//(float)Ki/10000;
    pid->Kd = 1;//Kd;
}

uint8_t UpdatePID(PIDController *pid, uint8_t cn){
 int16_t error, derivative;
  // Вычисление ошибки
  error = set[cn] - ds[cn].pvT;
  // Пропорциональная составляющая
  pid->pPart = (float)error * pid->Kp;
  // Интегральная составляющая
  pid->iPart += (float)error * pid->Ki;// * dt;
  // Дифференциальная составляющая
  derivative = (error - pid->prev_error);// / dt;
  pid->dPart = pid->Kd * derivative;
  // Сохраняем текущую ошибку для следующего вызова
  pid->prev_error = error;
  // Суммарное управляющее воздействие
  pid->output = pid->pPart + pid->iPart + pid->dPart;
  // Ограничение выходного значения и антивиндовинг
  if (pid->output > 100) pid->output = 110;
  else if (pid->output < 0) pid->output = 0;
  if (pid->pPart >= 100) pid->iPart = 0; // Сброс интеграла
  else if (pid->pPart <= -50) pid->iPart = 0; // Сброс интеграла

  error = pid->output;
  return (uint8_t)error;
}
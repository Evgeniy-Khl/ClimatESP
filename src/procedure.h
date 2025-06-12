#ifndef _PROCEDURE_H
#define _PROCEDURE_H

#include <Arduino.h>

typedef struct {
    float Ki, iPart, Kp, pPart;
    int32_t dPart, prev_error, output;
    uint16_t Kd;
} PIDController;

extern PIDController pid;
extern uint16_t set[];

void PID_Init(PIDController *pid);
uint8_t UpdatePID(PIDController *pid, uint8_t cn);
void permutation (char a, char b);
uint8_t sendToI2c(uint16_t val);

#endif /* _PROCEDURE_H */

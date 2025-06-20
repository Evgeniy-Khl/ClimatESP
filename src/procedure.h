#ifndef _PROCEDURE_H
#define _PROCEDURE_H
#include "main.h"

typedef struct {
    float Ki, iPart, Kp, pPart;
    int32_t output;
} PIDController;


extern PIDController pid[];
extern uint8_t seconds;

void PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki);
uint8_t UpdatePID(PIDController *pid, uint8_t cn);
uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);
void printConfig();
void saveConfig();
bool loadConfig();

#endif /* _PROCEDURE_H */

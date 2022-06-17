#ifndef _KMA210_H_
#define _KMA210_H_

#include "stm8s_conf.h"
#include "protocol_is.h"
#include "step_motor.h"

#define ANGLE_ERROR -1

#define MIN_ANGLE_KMA210    50          //5% Vdd = 51.2, но реальные значения меньше
#define MAX_ANGLE_KMA210    971         //95% Vdd = 972.8
#define MAX_DEGREE_KMA210   179.8       //макс. значение датчика в градусах
#define STEP_TO_SENSOR      1.537794    //(972.8-51.2)/599.3
#define COEF_ANGLE          0.1951      //(179.8 / 972.6-51.2)

void Init_KMA210(void);
void ReadKMA210(void);
s16 GetAngle_KMA210(void);
void ProgramKMA210(void);
u16 Read_Memory(void);
u16 ReadMemory(u8 uRegister);
bool WriteRegAngle(void);
bool SetToZero(void);
void PointerSetToZero(void);

#endif //_KMA210_H_

#ifndef _STEP_MOTOR_H_
#define _STEP_MOTOR_H_

#include "angle.h"
#include "signals.h"
#include "stm8s_conf.h"

bool GoToZero(void);
bool InitIS(void);
void StepUp(u16 steps);
void StepDown(u16 steps);
void SettingNewStep(void);
void VariablesReset (void);

#endif
/**
 * @authors
 * � ��������� �. �., 2012-2013
 * @brief ��������� ����
 * @file angle.h
 * @details
 * ������: ���
 * ���������������: STM8S208MB
 * �����: ��� 103.2
 */
 
#ifndef _ANGLE_H_
#define _ANGLE_H_

#include "kma210.h"
#include "stm8s_conf.h"
#include "signals.h"

#define SEMICIRCLE_STEPS    599.3           //179.8 �������� (����. �������� �������) � �����
#define CROSSING_RANGE1     597             //180 �������� (600 �����) ������� �������� �������
#define CROSSING_RANGE2     603             //�� ��������� ����������� � ���� �������
#define ERROR_MAXCOUNTER    10              //����. ���-�� ������ ��� ���������
#define ERROR_ANGLE         6 
#define ROUND(x) (x + 0.5)

void InitAngle(void);
s16 AngleToDegree(void);
s16 ReadAngle(void);
s16 Round(float fAng);
s16 GetAverageAngle(void);
void SyncCheck (void);
u8 ConvertAngle(u16 uSpeed,s16 iAngle);
void SaveAngle(u16 uSpeed,s16 iAngle,u16 uDiff);
u8 GetSendAngle(void);
u16 StepToAngleSensor(s16 iSteps);

#endif
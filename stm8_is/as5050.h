/**
 * @authors
 * � ��������� �. �., 2013
 * @brief ��������� ���� � ������� AS5050 �� ��������� SPI
 * @file as5050.h
 * @details
 * ������: ���
 * ���������������: STM8S208MB
 * �����: ��� 103.2
 */
 
#ifndef _AS5050_H_
#define _AS5050_H_

#define DEBUG_RS232

//����� ��������� � signals
#define AS5050_PORT GPIOC


// AS5050_PORT
//#define AS5050_SELECT_PIN GPIO_PIN_4  //�� ���� �
#define AS5050_CLK_PIN    GPIO_PIN_5
#define AS5050_OUT_DATA_PIN GPIO_PIN_6
#define AS5050_IN_DATA_PIN GPIO_PIN_7

#define MAX_ANGLE_AS5050 0x3FF
#define MAX_DEGREE_AS5050 359

void Init_AS5050(void);
s16 GetAngle_AS5050(void);

#endif
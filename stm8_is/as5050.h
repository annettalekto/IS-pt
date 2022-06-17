/**
 * @authors
 * © Степченко М. В., 2013
 * @brief Получение угла с датчика AS5050 по протоколу SPI
 * @file as5050.h
 * @details
 * Проект: ККМ
 * Микроконтроллер: STM8S208MB
 * Отдел: СКБ 103.2
 */
 
#ifndef _AS5050_H_
#define _AS5050_H_

#define DEBUG_RS232

//можно перенести в signals
#define AS5050_PORT GPIOC


// AS5050_PORT
//#define AS5050_SELECT_PIN GPIO_PIN_4  //на порт В
#define AS5050_CLK_PIN    GPIO_PIN_5
#define AS5050_OUT_DATA_PIN GPIO_PIN_6
#define AS5050_IN_DATA_PIN GPIO_PIN_7

#define MAX_ANGLE_AS5050 0x3FF
#define MAX_DEGREE_AS5050 359

void Init_AS5050(void);
s16 GetAngle_AS5050(void);

#endif
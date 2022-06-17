#ifndef _SIGNALS_H_
#define _SIGNALS_H_

#include "stm8s_tim1.h"
#include "stm8s_flash.h"
#include "step_motor.h"

#define USE_GLOBALS extern volatile /*page0*/ GLOBALS glob
#define BOOT_LDR_ADDR       0x487E

//����� ��������� ��
#define BUS_ONE_PORT		GPIOE//GPIOD	//�� ����� ���� ����� � ������ ����������
#define BUS_ZERO_PORT	    GPIOD//GPIOE
//���� ������ ��������� ����
#define BUTTON_PORT         GPIOB
//����� ���������� ���. ����.
#define TURNC_PORT          GPIOC
#define TURND_PORT          GPIOD
//KMA210 out/data
#define KMA210_PORT         GPIOF
//���� ���� ������ ������� �� KMA210
#define POWER_KMA210_PORT   GPIOB

//KMA210_PORT
#define KMA210_PIN          GPIO_PIN_4
//POWER_KMA210_PORT
#define POWER_KMA210_PIN    GPIO_PIN_1
//BUTTON_PORT  
#define BUTTON_PIN          GPIO_PIN_0
//���� ��������� ��
#define PIN_ONE		        GPIO_PIN_5
#define	PIN_ZERO	        GPIO_PIN_7

#define IN_ONE 		(BUS_ONE_PORT->IDR & (vu8)PIN_ONE)
#define IN_ZERO 	(BUS_ZERO_PORT->IDR & (vu8)PIN_ZERO)

// 0 �� ����� �������� ����������� �������.
// ������� ����� �������� �������������, 
// ����� 0 ���������� ���������� ��������, � ��������
#define IN_N_ONE 	!(BUS_ONE_PORT->IDR & (vu8)PIN_ONE)
#define IN_N_ZERO 	!(BUS_ZERO_PORT->IDR & (vu8)PIN_ZERO)

//������� � ����� ������ ���, ���� 1, �� �����
#define OUT_MODE_ONE	(BUS_ONE_PORT->DDR &(vu8)PIN_ONE)
#define OUT_MODE_ZERO	(BUS_ZERO_PORT->DDR &(vu8)PIN_ZERO)

#define IN_BUTTON !(BUTTON_PORT->IDR & (vu8)BUTTON_PIN)//����� ������ ������ - ���� ����� �� �����

// KMA210_PORT
#define IN_KMA210   (KMA210_PORT->IDR & (vu8)KMA210_PIN)

//�������� ���� ���������� ���. ����. � ����
#define TURNC_RESET 0xE1
#define TURND_RESET 0xE2
#define RESET_TURN()   TURNC_PORT->ODR &= TURNC_RESET;\
                       TURND_PORT->ODR &= TURND_RESET

//������� �������� ����������� �������� �������� ������� 1
#define COUNTER_HI TIM1->CNTRH
#define COUNTER_LO TIM1->CNTRL

//��������� �������� ����������� �������� ������� 1
#define GET_COUNT_VAL(res) { res = TIM1->CNTRH<<8; res += TIM1->CNTRL; }
//��������� �������� ����������� �������� ������� 1 � �������
#define TIME_TO_SEC() ((u32)GetCounterValue() / (u32)3000000)

#define WRITE_0(Port, Pin) Port->ODR &= (u8)(~Pin)
#define WRITE_1(Port, Pin) Port->ODR |= (u8)( Pin)

/*! \brief �������� ���������� �������.*/
/*! \arg \c exp ������� (������ ���������� bool).
 *  \arg \c timeout ����� �������� ������� (� 1/3000000 �).
 *  \arg \c res ������, � ������� ����� ������� ������� �����������
 *  ������� �� �������� ����� (bool).
 */
#define WAIT_EVENT(exp, timeout, res) { u16 time_off; u16 new_time; GET_COUNT_VAL(time_off); time_off+=timeout; \
do {(res) = (exp); if (res) break; GET_COUNT_VAL(new_time);} while(new_time < time_off); }

#define SET_STATUS(status,num_bit)      ( status &= (u8)~num_bit )
#define RESET_STATUS(status,num_bit)    ( status |= (u8) num_bit )

#define ENABLE_CODE         (0x10AD & 0x7FFF)//0x10AD
#define DISABLE_CODE        (0xDEAD & 0x7FFF)//0x5EAD


void ResetCounterTime(void);
void CounterTimerStart(void);
void CounterTimer2Start(void);
u32 GetCounterValue(void);
u32 GetCounter2Value(void);
void InitSignals(void);
void DelaySec (u8 uSec);
void PointerReset(void);
void delay(u32 Count);
void EnableBootloader(void);
void DisableBootloader(void);
void EnableUART(void);
void DisableUART(void);
void ControlBootloader (void);
void ControlUART(void);
int timer_expired(struct timer *t);
int timer_get(struct timer *t);
void timer_set(struct timer *t, int usecs);
unsigned int GetTickCount(void);
void ConnectCheck (void);

#endif
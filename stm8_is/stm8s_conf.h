/**
  ******************************************************************************
  * @file stm8s_conf.h
  * @brief This file is used to configure the Library.
  * @author STMicroelectronics - MCD Application Team
  * @version V1.1.1
  * @date 06/05/2009
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  * @image html logo.bmp
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM8S_CONF_H
#define __STM8S_CONF_H

/* Includes ------------------------------------------------------------------*/

#include "stm8s.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "pt.h"
#include "signals.h"
#include "uart3.h"

/* Exported types ------------------------------------------------------------*/

struct timer { int start, interval; };

struct pt_arg
{
  struct pt pt;
  struct timer timer;
};

typedef enum
{
	ST_ROLL_TO_ZERO	= (u8)0x01,		//Установка нуля,
                                    //бит равен нулю при докрутке до нуля.
	ST_GO_UP		= (u8)0x02,		//Идем вверх,
                                    //бит равен нулю при движении стрелки вверх.
	ST_OVERFLOW		= (u8)0x04,		//Переполнение,
                                    //бит равен нулю, если принята скорость свыше SPEED_MAX.
	ST_MOVING		= (u8)0x08,		//Движение стрелки,
                                    //бит равен нулю при движении стрелки.
	ST_TIMEOUT		= (u8)0x10,		//Таймаут,
                                    //бит равен нулю, если был превышен IS_TIMEOUT при приеме/передаче.
	ST_SYNC			= (u8)0x20,		//Синхронизация,
                                    //бит равен нулю, если произошла ошибока синхронизации. Сброс стрелки ниже нуля.
                                    //(ош.возникает, если шаги переведенные в градусы отличаются от угла считанного с датчика).
	ST_NO_CONNECT	= (u8)0x40,		//Нет связи,
                                    //бит равен нулю при отсутствии связи с ЦП больше TIME_OUT_IS.
	ST_CRC_ERROR	= (u8)0x80		//Бит контрольной суммы. Дополнение до нечетного.
} Status_TypeDef;

typedef struct 
{
    bool bSetNewStep;   //признак того что принята новая скорость 
    u16 uFixedStep;     //установленная скорость
    u16 uNewStep;       //скорость принятая из ЦП (абс. значение шагов от нуля на которое нужно перейти 150км/ч = 760 шагов)
    u8 uAngleArray[3];  //угол без мл.разряда (u8), угол с без округления (u16 для отлад. реж.)
    u8 uErrorCounter;
    Status_TypeDef StatusIS;

    bool bEnableUART;
    unsigned int uTickCount;//таймер для пп
    struct timer connect_is;//для отсчета обрыва связи
} GLOBALS;

/* Exported constants --------------------------------------------------------*/

#define STABILIZATION_TIME() delay(9000) 
#define ONE_SECOND           45                         /* см. TIM1 */
#define CONNECT_IS           3                          //(cек.) сброс при обрыве связи 
#define CONNECT_IS_TIMER     (CONNECT_IS * ONE_SECOND)  //3 cек. сброс при обрыве связи 

/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */ 

/* In the following line adjust the value of External High Speed oscillator (HSE)
   used in your application */
#if defined (STM8S208) || defined (STM8S207)
 #define HSE_VALUE ((u32)12000000) /* Value of the External oscillator in Hz*/
#else
 #define HSE_VALUE ((u32)12000000) /* Value of the External oscillator in Hz*/
#endif

/* Uncomment the line below to enable peripheral header file inclusion */

/************************************* ADC ************************************/
#if defined(STM8S105) || defined(STM8S103) || defined(STM8S903)
/* #define _ADC1 (1) */  
#endif /* (STM8S105) ||(STM8S103) || (STM8S903) */

#if defined(STM8S208) || defined(STM8S207)
#define _ADC2 (1) 
#endif /* (STM8S208) ||(STM8S207)  */

/************************************* AWU ************************************/
/* #define _AWU (1) */

/************************************* BEEP ***********************************/
/* #define _BEEP (1) */ 

/************************************* CLK ************************************/
#ifdef STM8S208
#define _CAN (1)
#endif /* (STM8S208) */

/************************************* CLK ************************************/
#define _CLK (1)

/************************************* EXTI ***********************************/
/* #define _EXTI (1) */

/******************************* FLASH/DATA EEPROM ****************************/
/* #define _FLASH (1) */

/********************************* OPTION BYTES *******************************/
/* #define _OPT (1) */

/************************************* GPIO ***********************************/
#define _GPIO (1)

/************************************* I2C ************************************/
/* #define _I2C  (1) */

/************************************* ITC ************************************/
/* #define _ITC (1) */

/************************************* IWDG ***********************************/
/* #define _IWDG (1) */

/************************************* RST ************************************/
/* #define _RST (1) */

/************************************* SPI ************************************/
/* #define _SPI (1) */

/************************************* TIM1 ***********************************/
/* #define _TIM1 (1) */

/************************************* TIM2 ***********************************/
#if defined(STM8S208) ||defined(STM8S207) ||defined(STM8S103) ||defined(STM8S105)
#define _TIM2 (1) 
#endif /* (STM8S208) ||(STM8S207)  || (STM8S103) || (STM8S105)  */

/************************************* TIM3 ***********************************/
#if defined(STM8S208) ||defined(STM8S207) ||defined(STM8S105)
/* #define _TIM3 (1) */
#endif /* (STM8S208) ||(STM8S207)  || (STM8S105) */

/************************************* TIM4 ***********************************/
#if defined(STM8S208) ||defined(STM8S207) ||defined(STM8S103) ||defined(STM8S105)
/* #define _TIM4 (1) */
#endif /* (STM8S208) ||(STM8S207)  || (STM8S103) || (STM8S105)  */

/************************************* TIM5 & TIM6 ****************************/
#ifdef STM8S903
/* #define _TIM5 (1) */
/* #define _TIM6 (1) */
#endif /* STM8S903 */ 

/************************************* UARTx **********************************/
#if defined(STM8S208) ||defined(STM8S207) ||defined(STM8S103) ||defined(STM8S903)
#define _UART1 (1) 
#endif /* (STM8S208) ||(STM8S207)  || (STM8S103) || (STM8S903)  */

#ifdef STM8S105
/* #define _UART2 (1) */
#endif /* STM8S105 */

#if defined(STM8S208) ||defined(STM8S207)
/* #define _UART3 (1) */
#endif /* (STM8S208) ||(STM8S207) */

/************************************* WWDG ***********************************/
/* #define _WWDG (1) */

/* Exported macro ------------------------------------------------------------*/

#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed. 
  *   If expr is true, it returns no value.
  * @retval : None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((u8 *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(u8* file, u32 line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */


/* Peripheral header file inclusion ******************************************/
#ifdef _ADC1
 #include "stm8s_adc1.h"
#endif /* _ADC1 */

#ifdef _ADC2
 #include "stm8s_adc2.h"
#endif /* _ADC2 */

#ifdef _AWU
 #include "stm8s_awu.h"
#endif /* _AWU */

#ifdef _BEEP
 #include "stm8s_beep.h"
#endif /* _BEEP */

#ifdef _CLK
 #include "stm8s_clk.h"
#endif /* _CLK */

#ifdef _EXTI
 #include "stm8s_exti.h"
#endif /* _EXTI */

#if defined(_FLASH) || defined(_OPT)
 #include "stm8s_flash.h"
#endif /* _FLASH/OPT */

#ifdef _GPIO
 #include "stm8s_gpio.h"
#endif /* _GPIOx */

#ifdef _I2C
 #include "stm8s_i2c.h"
#endif /* _I2C */

#ifdef _ITC
 #include "stm8s_itc.h"
#endif /* _ITC */

#ifdef _IWDG
 #include "stm8s_iwdg.h"
#endif /* _IWDG */

#ifdef _RST
 #include "stm8s_rst.h"
#endif /* _RST */

#ifdef _SPI
 #include "stm8s_spi.h"
#endif /* _SPI */

#ifdef _TIM1
 #include "stm8s_tim1.h"
#endif /* _TIM1 */

#ifdef _TIM2
 #include "stm8s_tim2.h"
#endif /* _TIM2 */

#ifdef _TIM3
 #include "stm8s_tim3.h"
#endif /* _TIM3 */

#ifdef _TIM4
 #include "stm8s_tim4.h"
#endif /* _TIM4 */

#ifdef _TIM5
 #include "stm8s_tim5.h"
#endif /* _TIM5 */

#ifdef _TIM6
 #include "stm8s_tim6.h"
#endif /* _TIM6 */

#ifdef _UART1
 #include "stm8s_uart1.h"
#endif /* _UART1 */

#ifdef _UART2
 #include "stm8s_uart2.h"
#endif /* _UART2 */

#ifdef _UART3
 #include "stm8s_uart3.h"
#endif /* _UART3 */

#ifdef _WWDG
 #include "stm8s_wwdg.h"
#endif /* _WWDG */

#ifdef _CAN
 #include "stm8s_can.h"
#endif /* _CAN */

#endif /* __STM8S_CONF_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

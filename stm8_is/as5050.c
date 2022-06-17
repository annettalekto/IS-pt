/**
 * @authors
 * © Степченко М. В., 2013
 * @brief Получение угла с датчика AS5050 по протоколу SPI
 * @file as5050.c
 * @details
 * Проект: ККМ
 * Микроконтроллер: STM8S208MB
 * Отдел: СКБ 103.2
 */

#include "main.h"
#include "as5050.h"

#define PARITY_BIT 0x0001
#define ERROR_BIT  0x0002
#define WRITE_BIT  0x8000

typedef bool AS_RWTYPE;
#define AS_READ   false
#define AS_WRITE  true



// Регистры AS5050
#define POWER_ON_RESET  0x3F22 /* чтение/запись */
#define SOFTWARE_RESET  0x3C00 /* запись */
#define MASTER_RESET    0x33A5 /* запись */
#define CLR_ERROR_FLAG  0x3380 /* чтение */
#define NO_OPERATION    0x0000 /* запись */
#define AUTO_GAIN_CTRL  0x3FF8 /* чтение/запись */
#define ANGULAR_DATA    0x3FFF /* чтение */
#define ERROR_STATUS    0x33A5 /* чтение */

#define CS_1	        WRITE_1(AS5050_SELECT_PORT, AS5050_SELECT_PIN)	
#define CS_0		    WRITE_0(AS5050_SELECT_PORT, AS5050_SELECT_PIN)

static u16 SPI_ReadWrite(u16 uPackage)
{ 
  u16 uResult = 0;
  u8  uHigh;
  
  CS_0;   //на шине только одно устройство
  
  // Послать 16 бит (текущая команда) 
  
  while ((SPI->SR & (u8)SPI_FLAG_TXE) == RESET) { ; }
  SPI->DR = (u8)(uPackage >> 8);

  while ((SPI->SR & (u8)SPI_FLAG_RXNE) == RESET) { ; }
  uHigh = SPI->DR;
  
  while ((SPI->SR & (u8)SPI_FLAG_TXE) == RESET) { ; }
  SPI->DR = (u8)(uPackage & 0x00FF); 
  
  while ((SPI->SR & (u8)SPI_FLAG_RXNE) == RESET) { ; }
  uResult = SPI->DR; 
  
  CS_1;
  
  uResult |= (u16)uHigh << 8; 
  //printf(" spi:%20d",uResult);
  return uResult;
}


// Проверка чётности
static bool CheckEvenParity(u16 uWord)
{
  u8 uOnes = 0;
  u8 uMask = 1;
  u8 i;
  
  for (i = 0; i < 16; i++)
  {
    if (uWord & (uMask << i))
      uOnes++;
  }    

  return (0 == (uOnes % 2));
}

// количество бит "1" должно быть чётным
static void SetEvenParity(u16 *uWord)
{
  if (NULL == uWord)
    return;
  
  if (!CheckEvenParity(*uWord))
  {
    *uWord |= PARITY_BIT;
  }  
}

/**
  * @brief Проверить правильность полученных данных
  * @param[in]  uData Данные
  * @param[out] 
  * @retval
  * false в случае ошибки, иначе true
*/
static bool CheckData(u16 uData)
{ 
  bool bResult = true;
  
  if (!CheckEvenParity(uData))
  {
    #ifdef DEBUG_RS232
    putchar('p'); //Ошибка чётности
    #endif
    bResult = false;
  }   
  
  if (uData & ERROR_BIT)
  {
    #ifdef DEBUG_RS232
    putchar('-'); // Появился бит ошибки
    #endif
    bResult = false;
  } else
  {
    #ifdef DEBUG_RS232
    //putchar('+'); // Нет ошибки
    #endif
  }
  
  #ifdef DEBUG_RS232
  if (uData & 0x1000)
  {    
    putchar('_'); // Магнитное поле слишком низкое
  }

  if (uData & 0x2000)
  {
    putchar('^'); // Магнитное поле слишком высокое
  }
  #endif    
  
  return bResult;
}

/**
  * @brief Формирует команду для отправки
  * @param[in]  uAddress Адрес
  * @param[in] bWrite 1 - команда на запись; 0 - команда на чтение
  * @retval
  * Сформированная команда
*/
static u16 MakeCommand(u16 uAddress, AS_RWTYPE bWrite)
{
  u16 uResult;  
  
  uResult = uAddress << 1;

  if (bWrite)
    uResult &= ~WRITE_BIT;
  else
    uResult |=  WRITE_BIT;
  
  SetEvenParity(&uResult);
  
  return uResult;
}

/**
  * @brief Формирует пакет с данными для отправки
  * @param[in]  uData данные
  * @retval
  * Сформированный пакет
*/
static u16 MakePackage(u16 uData)
{
  u16 uResult;    
  uResult = uData << 2;
  SetEvenParity(&uResult);  
  return uResult;
}

/**
  * @brief Получить данные датчика (код угла)
  * @par
  * Параметры отсутствуют
  * @retval Код угла, либо -1 при ошибке
  */
s16 GetAngle_AS5050(void)
{
  s16 iAngle;
  u16 uData;

  GPIO_Init(AS5050_SELECT_PORT,AS5050_SELECT_PIN,GPIO_MODE_OUT_PP_HIGH_FAST);//ss

  SPI_ReadWrite(MakeCommand(ANGULAR_DATA, AS_READ));
  uData = SPI_ReadWrite(MakeCommand(NO_OPERATION, AS_WRITE));
  //printf(" A:%10d",uData);
  if (CheckData(uData))
  {    
    iAngle = (uData >> 2) & MAX_ANGLE_AS5050;
    //printf("Angle:%10d\n",iAngle);
  } else
  {
    SPI_ReadWrite(MakeCommand(CLR_ERROR_FLAG, AS_READ));
    iAngle = ANGLE_ERROR;
  }
 
  GPIO_Init(AS5050_SELECT_PORT,AS5050_SELECT_PIN,GPIO_MODE_IN_PU_NO_IT);//GPIO_MODE_OUT_PP_HIGH_FAST  

  return iAngle;
}

/**
  * @brief Инициализация чтения по протоколу SPI
  * @par
  * Параметры отсутствуют
  * @retval
  * Возвращаемое значение отсутствует
*/
void Init_AS5050(void) 
{ 
  SPI_DeInit(); 

  GPIO_Init (AS5050_PORT, AS5050_OUT_DATA_PIN, GPIO_MODE_OUT_PP_HIGH_FAST); 
  GPIO_Init (AS5050_PORT, AS5050_IN_DATA_PIN, GPIO_MODE_IN_FL_NO_IT); 
  GPIO_Init (AS5050_PORT, AS5050_CLK_PIN, GPIO_MODE_OUT_PP_HIGH_FAST); 
  // Для AS5050 нужен интерфейс (CPOL = 0 / CPHA = 1)
  SPI_Init 
  (
    // 64 
    SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_256, SPI_MODE_MASTER, 
    SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_2EDGE,                    
    SPI_DATADIRECTION_2LINES_FULLDUPLEX, SPI_NSS_SOFT, 0x00  
  ); 
  
  SPI_Cmd(ENABLE); 
  
  SPI_ReadWrite(MakeCommand(SOFTWARE_RESET, AS_WRITE));
  SPI_ReadWrite(MakePackage(0x00));  
} 

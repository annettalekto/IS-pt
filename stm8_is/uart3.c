#include "uart3.h"

//если в putty кракоз€бры пробуем 14400 и 38400
void Init_UART(void) 
{   
  UART3_DeInit(); 
  // 115200 8N1 
  UART3_Init((u32)19200, UART3_WORDLENGTH_8D, UART3_STOPBITS_1, UART3_PARITY_NO,   
              UART3_MODE_TXRX_ENABLE);   //UART1_SYNCMODE_CLOCK_DISABLE
  
  UART3_Cmd(ENABLE);
}

void DeInit_UART(void)
{
    UART3_DeInit();
}

BitStatus GPIO_ReadOutputPin(GPIO_TypeDef* GPIOx, GPIO_Pin_TypeDef GPIO_Pin) 
{ 
  return ((BitStatus)(GPIOx->ODR & (vu8)GPIO_Pin)); 
}

/**
  * @brief Retargets the C library printf function to the USART.
  * @param[in] c Character to send
  * @retval char Character sent
  * @par Required preconditions:
  * - None
  */
PUTCHAR_PROTOTYPE
{
  /* Write a character to the UART1 */
  UART3_SendData8(c);
  /* Loop until the end of transmission */
  while (UART3_GetFlagStatus(UART3_FLAG_TXE) == RESET);

	return (c);
}
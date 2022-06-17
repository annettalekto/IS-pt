#include "signals.h"

//#pragma WARNINGLEVEL(0)                 
at 0x4000 eeprom u16 BootloaderEEPROM;  //переменная для включения Bootloader
at 0x4004 eeprom u16 UartEEPROM;        //переменная для вкл. UART (+отладочного режима)       
                                        //отладочный режим - передача по ИС-протоколу значения угла 
                                        //в двух байтах, вывод отл. инф. в UART

volatile /*page0*/ GLOBALS glob;
static u16 g_uOverflowCount = 0;

void InitSignals(void)
{
    //кнопка настройки нуля
    GPIO_Init(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_IN_FL_NO_IT);
    //входы управления шаг.двиг. 
    GPIO_Init(TURNC_PORT, (GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4), GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(TURND_PORT, (GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4), GPIO_MODE_OUT_PP_LOW_FAST);
    //сбросить в ноль выходы управления шаг.двиг.
    RESET_TURN();
}

//============================ Настройка таймера 1 ============================//
//=============================================================================//

//получить значение внутреннего счетчика таймера
u32 GetCounterValue(void)
{
    u32 uResult;
    uResult  = COUNTER_HI << 8;
    uResult += COUNTER_LO;
    uResult += (u32)U16_MAX * (u32)g_uOverflowCount; 
    return uResult;
}

//сбросить внутренний счетчик таймера
void ResetCounterTime(void)
{
    COUNTER_HI = 0;
    COUNTER_LO = 0;
    g_uOverflowCount = 0;
}

/**
  * @brief Включить таймер № 1.
  * @par
  * Параметры отсутствуют
  * @retval
  * Возвращаемое значение отсутствует
*/
void CounterTimerStart(void)
{  
    TIM1_DeInit();  
    //12000000/4 = 3000000; 1 сек/3000000 = 0.33 мкс такт
    TIM1_TimeBaseInit(4, TIM1_COUNTERMODE_UP, U16_MAX, 0);          
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    COUNTER_HI = 0;
    COUNTER_LO = 0;
    g_uOverflowCount = 0;
    TIM1_Cmd(ENABLE);
}

/**
  * @brief Прерывание внутреннего счётчика. Таймер № 1.
  * @par
  * Параметры отсутствуют
  * @retval
  * Возвращаемое значение отсутствует
*/
void TIM1_UPD_OVF_TRG_BRK_IRQHandler(void) interrupt 11
{
  if (TIM1_GetITStatus(TIM1_IT_UPDATE))
  {
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);   
    g_uOverflowCount++;                                 //при 30 сек выдает 38
    glob.uTickCount++;  //12000000/4 = 3000000 / 65536 = 45.776 прерываний в сек.
  }
}

//============================ Для отсчета времени ============================//
//=============================================================================//

//при установке на 30 сек. дает зад 37-38 сек
void DelaySec (u8 uSec)
{
    struct time tim;

    timer_set(&tim, uSec*ONE_SECOND);
    while(!timer_expired(&tim))
    {
        _nop_();
    }
}

unsigned int GetTickCount(void)
{
  return glob.uTickCount;
}

int clock_time(void)
{
  return (int)GetTickCount();
}

int timer_expired(struct timer *t)
{ 
  return (int)(clock_time() - t->start) >= (int)t->interval; 
}

int timer_get(struct timer *t)
{
  return (int)(clock_time() - t->start);
}

void timer_set(struct timer *t, int interval)
{
  t->interval = interval; t->start = clock_time(); 
}

void ConnectCheck (void)
{
    if(timer_expired(&glob.connect_is))
    {
        SET_STATUS(glob.StatusIS,ST_NO_CONNECT);

        if(glob.bEnableUART)
            printf("\t!!!Нет связи по ИС-протоколу более %d сек.\r\n",(u8)CONNECT_IS);
        PointerReset();
    }
}

//=============================================================================//
//=============================================================================//

//Отпустить стрелку
void PointerReset(void)
{
    struct timer tim;
    RESET_TURN();                 //сброс стрелки

    timer_set(&tim, 2*ONE_SECOND);//ждать когда стрелка упадет
    while(!timer_expired(&tim))
    { ; }
    
    InitIS();
}

//Если UART включен - включен режим отлаки
void EnableUART (void)
{
    FLASH_DeInit();
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

        UartEEPROM = ENABLE_CODE;

    FLASH_Lock(FLASH_MEMTYPE_DATA);
    ControlUART();
}

void DisableUART(void)
{
    if(glob.bEnableUART)
        printf("UART disable\r\n");
    glob.bEnableUART = false;

    FLASH_DeInit();
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

        UartEEPROM = DISABLE_CODE;

    FLASH_Lock(FLASH_MEMTYPE_DATA);

    DeInit_UART();
}

void ControlUART(void)
{
    if(UartEEPROM == ENABLE_CODE)
        glob.bEnableUART = true;
    else
        glob.bEnableUART = false;

    #ifdef UART_ENABLE
    glob.bEnableUART = true;
    #endif
    
    if(glob.bEnableUART != true)
        return;
        
    Init_UART();
    UART_CLEAR;
    
    printf("ПО ИС2 версия 5(ПП)\r\n");
    printf("Frequency: %.3f MHz\r\n", ((float)CLK_GetClockFreq())/1000000.);
    if(BootloaderEEPROM == ENABLE_CODE)
    {
        printf("Bootloader enable\r\n\r\n");
    }
    else
    {
        printf("Bootloader disable\r\n\r\n");
    }    
}

//Включить bootloader - после перезагрузки,
//перезагрузить чтобы вступило в силу 
void EnableBootloader(void)
{
    FLASH_DeInit();
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

        BootloaderEEPROM = ENABLE_CODE;

    FLASH_Lock(FLASH_MEMTYPE_DATA);
    ControlBootloader();
}

//Выключить bootloader - после перезагрузки,
//перезагрузить чтобы вступило в силу 
void DisableBootloader(void)
{
    FLASH_DeInit();
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

        BootloaderEEPROM = DISABLE_CODE;

    FLASH_Lock(FLASH_MEMTYPE_DATA);
    ControlBootloader();
}

void ControlBootloader (void)
{
    bool bEnable = false;

    if(BootloaderEEPROM == ENABLE_CODE)
        bEnable = true;
    else
        bEnable = false;
    
    #ifdef BOOTLOADER_ENABLE
    bEnable = true;
    #endif
    
    //открыть возможность записи в EEPROM
    FLASH_DeInit();  
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

    if(bEnable)
    {
        //включить Bootloader
        if (FLASH_ReadOptionByte(BOOT_LDR_ADDR) != 0x55AA)
        {
            FLASH_ProgramOptionByte(BOOT_LDR_ADDR, 0x55);
        }
    }
    else    //(BootloaderEEPROM == DISABLE_CODE)
    {
        // выключить Bootloader
        if(FLASH_ReadOptionByte(BOOT_LDR_ADDR) != 0x00FF)
        {
            FLASH_EraseOptionByte(BOOT_LDR_ADDR);      
        }
    }

    FLASH_Lock(FLASH_MEMTYPE_DATA);
}

void delay(u32 Count)
{
    while(Count--)
    {
        _nop_();
    }
}
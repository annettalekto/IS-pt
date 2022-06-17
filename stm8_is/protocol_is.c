#include "protocol_is.h"

#define IS_TIMEOUT          600                 //200 мкс таймаут на ответ/прием по  уцак-протоколу
#define TIME_RECEIVE        9000                //таймаут 3(мс)

USE_GLOBALS;
bool g_bChangeSpeed = false;
u8 g_uDataByte = 0;
u16 g_uReprogramCode = 0;

//Ќастройка внешнего прерывани€ на E5
void InitProtocolIS(void)
{
    GPIO_Init(BUS_ZERO_PORT,PIN_ZERO,GPIO_MODE_IN_PU_NO_IT);              
    GPIO_Init(BUS_ONE_PORT,PIN_ONE,GPIO_MODE_IN_PU_IT);                     
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_FALL_ONLY);
}

//ѕолучить бит
bool GetBit(bool *bDataBit)	
{
	bool bResult = false;

	if(IN_N_ONE)                                //Ўƒ в нуле
	{
        BUS_ZERO_PORT->DDR |= PIN_ZERO;         //на выход
		WRITE_0(BUS_ZERO_PORT,PIN_ZERO);        //дл€ подтверждени€ опускаем линию нулей
		WAIT_EVENT(IN_ONE,IS_TIMEOUT,bResult);  //ожидаем когда лини€ Ўƒ поднимитс€
        if(bResult)
        {
            WRITE_1(BUS_ZERO_PORT,PIN_ZERO);    //поднимаем линию нулей в ответ 
            *bDataBit = true;                   //т.о.была передана единица
            bResult = true;                     //если не был передан бит в течении IS_TIMEOUT,
        }                                       //результат false (и прекращение прием/передачи)
        BUS_ZERO_PORT->DDR &= (u8)(~PIN_ZERO);  //на вход
	}   
	else 
	{
		if(IN_N_ZERO)                           //clk в нуле
		{
            BUS_ONE_PORT->DDR|=PIN_ONE;
			WRITE_0(BUS_ONE_PORT,PIN_ONE);
			WAIT_EVENT(IN_ZERO,IS_TIMEOUT,bResult);
            if(bResult)
            {    
                WRITE_1(BUS_ONE_PORT,PIN_ONE);
                *bDataBit=false;
                bResult = true;
            }
            BUS_ONE_PORT->DDR &= (u8)(~PIN_ONE);
		}
 	}
	return bResult;
}

//ѕрин€ть байт, положить рез. в глоб.переменную 
bool GetByte(void)
{
    bool bResult = false;
	bool bDataBit = false;	
    u8 uBuf = 0;
    u8 uMask = 0x80;                            //цикл приема бит (начина€ со старшего)
	
	while(uMask)
	{
		WAIT_EVENT(GetBit((bool*)&bDataBit),TIME_RECEIVE,bResult);//ожидаем передачи одного из битов
		if(!bResult) break;                     //если не передан бит в течении IS_TIMEOUT
        
		if(bDataBit) uBuf |= uMask;             //фиксируем единичку
		uMask = uMask >> 1;
	}
	if (bResult) g_uDataByte = uBuf;            //если все нормально, то фиксируем данные
	return bResult;
}

//ѕереслать байт
bool SendByte(u8 uData)
{
	bool bResult = false;
	bool bPutBit = false;
	u8 uMask = 0x80;                            // цикл передачи бит (начина€ со старшего)

	while(uMask)
	{
		bPutBit = (uData & uMask);              // выдел€ем очередной бит
		
		if(bPutBit)
		{
            BUS_ONE_PORT->DDR |= PIN_ONE;       // ставим линию единиц (Ўƒ)в 0
			WRITE_0(BUS_ONE_PORT,PIN_ONE);
            BUS_ZERO_PORT->DDR &= (u8)(~PIN_ZERO);// ожидаем 0 по обратной линии  
			WAIT_EVENT(IN_N_ZERO,IS_TIMEOUT,bResult);
            if(!bResult) break;
        
			disableInterrupts();
			WRITE_1(BUS_ONE_PORT,PIN_ONE);      // востанавливаем линию в 1
			WAIT_EVENT(IN_ZERO,IS_TIMEOUT,bResult);// ожидаем 1 по обратной линии
		}
		else
		{
            BUS_ZERO_PORT->DDR |= PIN_ZERO;
			WRITE_0(BUS_ZERO_PORT,PIN_ZERO);
            BUS_ONE_PORT->DDR &= (u8)(~PIN_ONE);
			WAIT_EVENT(IN_N_ONE,IS_TIMEOUT,bResult);
			if(!bResult) break;
        
			disableInterrupts();
			WRITE_1(BUS_ZERO_PORT,PIN_ZERO);
			WAIT_EVENT(IN_ONE,IS_TIMEOUT,bResult);
		}
        enableInterrupts();
		if(!bResult) break;
		uMask = uMask >> 1;
	}	
		return bResult;
}

//¬ статусном байте должно быть нечетное кол-во единиц.
//ƒобавить бит четности, если нужно
u8 FormatStatusIS(void)
{
    u8 i, uStatus;
    u8 uMask = 0x01;
    u8 uBuf = 0;

    //проверка на четность
    uStatus = glob.StatusIS;
    for(i=0; i<7; i++)
    {	
        if(uStatus&(uMask<<i))
        {
            uBuf++;
        }
    }
    if(uBuf%2 == 0)
    {
        uStatus |= ST_CRC_ERROR;
    } 
    return uStatus;
}

//ѕроверка кода на вкл/выкл UART и Bootloader
bool CheckReprogramCommand(u16 uStep)
{
    bool bResult = true;

    switch(uStep)
    {
        case REPROGRAM_CODE_1:
            g_uReprogramCode = REPROGRAM_CODE_1;
        break;
        
        case REPROGRAM_CODE_2: 
            if (REPROGRAM_CODE_1 == g_uReprogramCode)
                g_uReprogramCode = REPROGRAM_CODE_2;
        break;
            
        case UART_CODE:
            if (REPROGRAM_CODE_2 == g_uReprogramCode)
                g_uReprogramCode = UART_CODE;
        break;
            
        case BOOTLOADER_CODE:
            if (REPROGRAM_CODE_2 == g_uReprogramCode)
                g_uReprogramCode = BOOTLOADER_CODE;
        break;  
            
        case ENABLE_CODE:
            if (UART_CODE == g_uReprogramCode)
            {
                g_uReprogramCode = ENABLE_CODE;//код может придти два раза enable и disale
                EnableUART();
            }
            else if (BOOTLOADER_CODE == g_uReprogramCode)
            {
                g_uReprogramCode = ENABLE_CODE;
                EnableBootloader();
            }
        break;

        case DISABLE_CODE:
            if (UART_CODE == g_uReprogramCode)
            {
                g_uReprogramCode = DISABLE_CODE;//код может придти два раза enable и disale
                DisableUART();
            }
            else if (BOOTLOADER_CODE == g_uReprogramCode)
            {
                g_uReprogramCode = DISABLE_CODE;
                DisableBootloader();
            }
        break;
            
        default:
            g_uReprogramCode = 0;
            bResult = false;
        break;
    }
    return bResult;
}

void EXTI_PORTE_IRQHandler(void) interrupt 7
{
    bool bSession = false;
    u8 uAngle = 0, uStatus = 0;
    u16 uStep = 0;

    BUS_ONE_PORT->CR2 &= (u8)(~PIN_ONE);           //сбросить внешнее прерывание                   
    ResetCounterTime();

    if(GetByte())
    {    
        uStep = g_uDataByte << 8;
        if(GetByte())
        {    
            uStep |= g_uDataByte;  
            uStep = uStep & 0x7FFF;                //15 бит скорости без стартового бита
                                                   //передать два байта
            if(glob.bEnableUART)                   //в режиме отладки вместо статуса
                uStatus = glob.uAngleArray[1];     //отправл€ем ст. байт данных дат.угла    
            else
                uStatus = FormatStatusIS();
            if(SendByte(uStatus))
            {
                if(glob.bEnableUART)               //в режиме отладки отправл€ем ст. байт
                    uAngle = glob.uAngleArray[2];  //данных датчика угла
                else
                    uAngle = glob.uAngleArray[0];
                bSession = SendByte(uAngle);
            }
        }
    }
    GPIO_Init(BUS_ZERO_PORT,PIN_ZERO,GPIO_MODE_IN_PU_NO_IT);//восстановить внешнее прерывание
    GPIO_Init(BUS_ONE_PORT,PIN_ONE,GPIO_MODE_IN_PU_IT);

    if(!bSession)
    {
        glob.bSetNewStep = false;
        SET_STATUS(glob.StatusIS,ST_TIMEOUT);      //прекратить прием/передачу и ждать нового сеанса св€зи
    }       
    else
    {
        timer_set(&glob.connect_is, CONNECT_IS_TIMER);//(cек.) сброс при обрыве св€зи 
        if(uStep > MAX_STEP)
        {
            if(CheckReprogramCommand(uStep))
                return;
            else
                uStep = MAX_STEP;                  //800 шагов предел, устанавливаем скорось
        }
        glob.uNewStep = uStep;
        if(glob.uNewStep != glob.uFixedStep)
            glob.bSetNewStep = true;               //разрешение на установку скорости

        RESET_STATUS(glob.StatusIS,ST_TIMEOUT);
    }
}
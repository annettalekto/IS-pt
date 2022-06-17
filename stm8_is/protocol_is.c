#include "protocol_is.h"

#define IS_TIMEOUT          600                 //200 ��� ������� �� �����/����� �� �����-���������
#define TIME_RECEIVE        9000                //������� 3(��)

USE_GLOBALS;
bool g_bChangeSpeed = false;
u8 g_uDataByte = 0;
u16 g_uReprogramCode = 0;

//��������� �������� ���������� �� E5
void InitProtocolIS(void)
{
    GPIO_Init(BUS_ZERO_PORT,PIN_ZERO,GPIO_MODE_IN_PU_NO_IT);              
    GPIO_Init(BUS_ONE_PORT,PIN_ONE,GPIO_MODE_IN_PU_IT);                     
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_FALL_ONLY);
}

//�������� ���
bool GetBit(bool *bDataBit)	
{
	bool bResult = false;

	if(IN_N_ONE)                                //�� � ����
	{
        BUS_ZERO_PORT->DDR |= PIN_ZERO;         //�� �����
		WRITE_0(BUS_ZERO_PORT,PIN_ZERO);        //��� ������������� �������� ����� �����
		WAIT_EVENT(IN_ONE,IS_TIMEOUT,bResult);  //������� ����� ����� �� ����������
        if(bResult)
        {
            WRITE_1(BUS_ZERO_PORT,PIN_ZERO);    //��������� ����� ����� � ����� 
            *bDataBit = true;                   //�.�.���� �������� �������
            bResult = true;                     //���� �� ��� ������� ��� � ������� IS_TIMEOUT,
        }                                       //��������� false (� ����������� �����/��������)
        BUS_ZERO_PORT->DDR &= (u8)(~PIN_ZERO);  //�� ����
	}   
	else 
	{
		if(IN_N_ZERO)                           //clk � ����
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

//������� ����, �������� ���. � ����.���������� 
bool GetByte(void)
{
    bool bResult = false;
	bool bDataBit = false;	
    u8 uBuf = 0;
    u8 uMask = 0x80;                            //���� ������ ��� (������� �� ��������)
	
	while(uMask)
	{
		WAIT_EVENT(GetBit((bool*)&bDataBit),TIME_RECEIVE,bResult);//������� �������� ������ �� �����
		if(!bResult) break;                     //���� �� ������� ��� � ������� IS_TIMEOUT
        
		if(bDataBit) uBuf |= uMask;             //��������� ��������
		uMask = uMask >> 1;
	}
	if (bResult) g_uDataByte = uBuf;            //���� ��� ���������, �� ��������� ������
	return bResult;
}

//��������� ����
bool SendByte(u8 uData)
{
	bool bResult = false;
	bool bPutBit = false;
	u8 uMask = 0x80;                            // ���� �������� ��� (������� �� ��������)

	while(uMask)
	{
		bPutBit = (uData & uMask);              // �������� ��������� ���
		
		if(bPutBit)
		{
            BUS_ONE_PORT->DDR |= PIN_ONE;       // ������ ����� ������ (��)� 0
			WRITE_0(BUS_ONE_PORT,PIN_ONE);
            BUS_ZERO_PORT->DDR &= (u8)(~PIN_ZERO);// ������� 0 �� �������� �����  
			WAIT_EVENT(IN_N_ZERO,IS_TIMEOUT,bResult);
            if(!bResult) break;
        
			disableInterrupts();
			WRITE_1(BUS_ONE_PORT,PIN_ONE);      // �������������� ����� � 1
			WAIT_EVENT(IN_ZERO,IS_TIMEOUT,bResult);// ������� 1 �� �������� �����
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

//� ��������� ����� ������ ���� �������� ���-�� ������.
//�������� ��� ��������, ���� �����
u8 FormatStatusIS(void)
{
    u8 i, uStatus;
    u8 uMask = 0x01;
    u8 uBuf = 0;

    //�������� �� ��������
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

//�������� ���� �� ���/���� UART � Bootloader
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
                g_uReprogramCode = ENABLE_CODE;//��� ����� ������ ��� ���� enable � disale
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
                g_uReprogramCode = DISABLE_CODE;//��� ����� ������ ��� ���� enable � disale
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

    BUS_ONE_PORT->CR2 &= (u8)(~PIN_ONE);           //�������� ������� ����������                   
    ResetCounterTime();

    if(GetByte())
    {    
        uStep = g_uDataByte << 8;
        if(GetByte())
        {    
            uStep |= g_uDataByte;  
            uStep = uStep & 0x7FFF;                //15 ��� �������� ��� ���������� ����
                                                   //�������� ��� �����
            if(glob.bEnableUART)                   //� ������ ������� ������ �������
                uStatus = glob.uAngleArray[1];     //���������� ��. ���� ������ ���.����    
            else
                uStatus = FormatStatusIS();
            if(SendByte(uStatus))
            {
                if(glob.bEnableUART)               //� ������ ������� ���������� ��. ����
                    uAngle = glob.uAngleArray[2];  //������ ������� ����
                else
                    uAngle = glob.uAngleArray[0];
                bSession = SendByte(uAngle);
            }
        }
    }
    GPIO_Init(BUS_ZERO_PORT,PIN_ZERO,GPIO_MODE_IN_PU_NO_IT);//������������ ������� ����������
    GPIO_Init(BUS_ONE_PORT,PIN_ONE,GPIO_MODE_IN_PU_IT);

    if(!bSession)
    {
        glob.bSetNewStep = false;
        SET_STATUS(glob.StatusIS,ST_TIMEOUT);      //���������� �����/�������� � ����� ������ ������ �����
    }       
    else
    {
        timer_set(&glob.connect_is, CONNECT_IS_TIMER);//(c��.) ����� ��� ������ ����� 
        if(uStep > MAX_STEP)
        {
            if(CheckReprogramCommand(uStep))
                return;
            else
                uStep = MAX_STEP;                  //800 ����� ������, ������������� �������
        }
        glob.uNewStep = uStep;
        if(glob.uNewStep != glob.uFixedStep)
            glob.bSetNewStep = true;               //���������� �� ��������� ��������

        RESET_STATUS(glob.StatusIS,ST_TIMEOUT);
    }
}
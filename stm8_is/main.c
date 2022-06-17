/**
 * ������: ��2
 * ������: 5 (��)
 * ���������������: STM8S207K6
 * �����: ��� 103.2
 */

#include "stm8s_conf.h"
#include "protocol_is.h"
#include "step_motor.h"
#include "angle.h"
#include "signals.h"

//��� ���������� Bootloader ��������������� ��������� �������
//~ #define BOOTLOADER_ENABLE   1
//~ #define UART_ENABLE         1

USE_GLOBALS;

void Init_CPU(void);

void main(void)
{
    disableInterrupts();

    ControlBootloader();
    Init_CPU();
    InitSignals();
    InitProtocolIS();
    CounterTimerStart();
    ControlUART();
    InitAngle();
    
    enableInterrupts();

    InitIS();  

    for(;;)
    {
        if(IN_BUTTON)
            PointerSetToZero();
        SettingNewStep();
        ConnectCheck();
        SyncCheck();
    }
}

/**
  * @brief ������������� ����������������
  * @par
  * ��������� �����������
  * @retval
  * ������������ �������� �����������
*/
void Init_CPU(void)		
{  
    u8 cnt = 0xFFFF;

    // ��������� �������� ������� ����������������
    CLK_DeInit();
    CLK_HSICmd(DISABLE);
    // ������� ��������, 12 ���  
    CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
    CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);  
  
    // ����� ������������ �������
    while (CLK_GetSYSCLKSource() != CLK_SOURCE_HSE)
    {
        if ( --cnt == 0 )
        return;
    }
}
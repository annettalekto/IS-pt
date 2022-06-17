inline void SetPin(unsigned char uPin);

#include "step_motor.h"

USE_GLOBALS;
volatile u8 g_uIndStep;	                //������� ����� � ������� StepArray

//24 ����(����) �������� ������ �������� ���������.
//��� �������� ������ ��������� �� 0 �� 23 ��������,
//��� �������� �����, ��������, �� 23 �� 0,
//�� �������� �� ���������� ���� StepArray[g_uIndStep+1].
//������������ ����� � �����:
//PC4 PC3 PC2 PC1 PD4 PD3 PD2 PD0
u8 StepArray[24] = {0xC0, 0xC1, 0xC2, 0xC3,
                    0x83, 0x43, 0x03, 0x07,
                    0x0B, 0x0F, 0x0E, 0x0D,
                    0x0C, 0x1C, 0x2C, 0x3C,
                    0x38, 0x34, 0x30, 0x70,
                    0xB0, 0xF0, 0xE0, 0xD0 };

void VariablesReset (void)
{
    glob.StatusIS = 0x7F;
    glob.uAngleArray[0] = 0; 
    glob.uAngleArray[1] = 0; 
    glob.uAngleArray[2] = 0; 
    glob.uFixedStep = 0;
    glob.uNewStep = 0;
    glob.uErrorCounter = 0;
    glob.bSetNewStep = false;
}
                             
//������������� ��, �������� �� ����
bool InitIS(void)
{
    g_uIndStep = 12;

    SET_STATUS(glob.StatusIS,ST_ROLL_TO_ZERO);
    if(GoToZero())
    {
        VariablesReset();
        return true;
    }
    return false;
}

//������� ��������� �����.
//������������ ����� � �����
//������ ����: PC4 PC3 PC2 PC1 PD4 PD3 PD2 PD0
inline void SetPin(unsigned char uPin)
{
	u8 uPinD = 0;
	u8 uPinC = 0;

    uPinD = (u8)(TURND_PORT->IDR & TURND_RESET);    //�������� ����������� ���� xxx0000x
    uPinC = (u8)(TURNC_PORT->IDR & TURNC_RESET);    //��������� �������� ��� ��������� xxx000x0
                                        
	uPinC  |= (u8)(uPin & 0xF0)>>3;	                //����� ������� ����� 11110000 � �����������(PC4 3 2 1)0001 1110
 	
	uPinD  |= (u8)(uPin & 0x0E)<<1;		            //����� ��� ���� ������� ����� 00001110 � �����������(PD4 3 2)0001 1100 
	uPinD  |= (u8)(uPin & 0x01);		            //��������� ��.��� 
    
    TURNC_PORT->ODR = (u8)uPinC; 			        //����������
	TURND_PORT->ODR = (u8)uPinD;
}

//������ ������
void StepUp(u16 uSteps)
{
    for(; uSteps > 0; uSteps--)
	{
        g_uIndStep = (g_uIndStep == 23) ? 0 : g_uIndStep+1; 
		SetPin(StepArray[g_uIndStep]);
        glob.uFixedStep++;
        delay(0xF);
	}
}

//������ �����
void StepDown(u16 uSteps)
{
    for(; uSteps > 0; uSteps--)
	{
        g_uIndStep = (g_uIndStep == 0) ? 23 : g_uIndStep-1; 
		SetPin(StepArray[g_uIndStep]);
        glob.uFixedStep--;
        delay(0xF);
   	}
}

//��������� �� ����
bool GoToZero(void)
{
    u16 uAngle = 0;
    u8 uCount = 80;

    StepUp(1);
    uAngle = GetAverageAngle();

    while(uCount && ((uAngle > 54)||(uAngle < 49)))
    {
        StepUp(1);
        uCount--;
        delay(0xff);
        uAngle = GetAverageAngle();
    
        if (IN_BUTTON)     
            return false;

    }
    if(uCount == 0)
    {
        //���� ���� �� ������ � ������� uCount ����� - ������
        SET_STATUS(glob.StatusIS,ST_SYNC);
        PointerReset();
        return false;
    }
    return true;  
}

void SettingNewStep(void)
{
    if (!glob.bSetNewStep)
        return;

    SET_STATUS(glob.StatusIS,ST_MOVING);      //������ - ��������
    if(glob.uNewStep > MAX_SPEED_SCALE)       //�������� �� ���������� ����.��������
        SET_STATUS(glob.StatusIS,ST_OVERFLOW);
    else
        RESET_STATUS(glob.StatusIS,ST_OVERFLOW);
    
    while(glob.uNewStep != glob.uFixedStep)   //��� ����� ���������� ���������� � ��������� uNewStep
    {
        s16 iStep = glob.uNewStep - glob.uFixedStep; 
        if(iStep > 0)
        {
            SET_STATUS(glob.StatusIS,ST_GO_UP);
            StepUp(1);
        }
        else
        {
            StepDown(1);
        }
        RESET_STATUS(glob.StatusIS,ST_GO_UP);
        RESET_STATUS(glob.StatusIS,ST_MOVING);
    }
    glob.bSetNewStep = false;
}
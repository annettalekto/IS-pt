#include "angle.h"
USE_GLOBALS;

/**
  * @brief ������������� �������� ���������� ����� � �������
  * @par
  * ��������� �����������
  * @retval
  * ������������ �������� �����������
*/
void InitAngle(void)
{
    u8 i;

    //������� kma210 � ������� ������
    GPIO_Init(POWER_KMA210_PORT, POWER_KMA210_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);
    
    Init_KMA210();  
    for(i=0; i<5; i++)
    {
        GetAngle_KMA210();
    }
}

//���� ��������� � �������� ������� ����
u16 StepToAngleSensor(s16 iSteps)
{
    float fResult;

    fResult = (float)iSteps;
    if(fResult >= SEMICIRCLE_STEPS)
    {
        fResult -= SEMICIRCLE_STEPS;    //������ 180 �������� 
    }
    fResult *= STEP_TO_SENSOR;
    fResult = (float)ROUND(MIN_ANGLE_KMA210 + fResult);

    return (u16)fResult;
}

//�������� ������� �������� ���� � ������� ��� �������� � �������
s16 GetAverageAngle(void)
{
    s16 iSumAngles = 0;
    s16 iAngle = ANGLE_ERROR;
    u8 i, uDiv = 0;

    for(i=0; i<8; i++)
    {
        iAngle = GetAngle_KMA210();
        if(iAngle != ANGLE_ERROR)
        {
            delay(0xff);
            iSumAngles += iAngle;
            uDiv++;
        }
    }
    iAngle = (uDiv > 0) ? (float)iSumAngles/uDiv : ANGLE_ERROR;
    return iAngle;
}

//��������� ���� ��� �������� � ��
void SaveAngle(u16 uSpeed,s16 iAngle,u16 uDiff)
{
    if(iAngle == ANGLE_ERROR)
    {
        glob.uAngleArray[0] = 0xFF;
        glob.uAngleArray[1] = 0xFF;
        glob.uAngleArray[2] = 0xFF;
    }

    glob.uAngleArray[0] = ConvertAngle(uSpeed, iAngle);
    glob.uAngleArray[1] = (u8)(uDiff >> 8);
    glob.uAngleArray[2] = (u8) uDiff;
}

//��������� ���� � ������� ��� �������� � ��
u8 ConvertAngle(u16 uSpeed,s16 iAngle)
{
    const u8 uError = 3;
    float fAngle;

    if (iAngle > MAX_ANGLE_KMA210-uError)       //>967
    {
        fAngle = MAX_DEGREE_KMA210;             //179.8 ��������
    }
    else if(iAngle < MIN_ANGLE_KMA210 + uError) //<53
    {
        if(uSpeed > SEMICIRCLE_STEPS - uError)
            fAngle = MAX_DEGREE_KMA210;
        else
            fAngle = 0;
    }
    else
    {    
        fAngle = (float) iAngle;
        fAngle = (fAngle - MIN_ANGLE_KMA210);   
        fAngle *= COEF_ANGLE;                   // �������������� � �������, 0-179
        if(uSpeed >= SEMICIRCLE_STEPS)          //������ ��������
            fAngle += MAX_DEGREE_KMA210;
    }

    return (u8) ROUND(fAngle); //ROUND(x)
}


//��������� ����� ������, ����� �� ����������� ��������� ��������
//STABILIZATION_TIME();
void SyncCheck (void)
{
	u16 uCalcAngle = 0, uDiffAngle = 0;
    s16 iSensorAngle = 0;
	
    STABILIZATION_TIME();   //��������� �� ������� �������

    if(glob.bSetNewStep)
        return;

	uCalcAngle = StepToAngleSensor(glob.uFixedStep);
	iSensorAngle = GetAverageAngle();
	uDiffAngle = abs(uCalcAngle - iSensorAngle);
    
    if(glob.bSetNewStep)//����� ������ ����� �������� � ���������� ���� 
        return;         //������������� ������� ��. ���� � ������. �����

	if(uDiffAngle > ERROR_ANGLE)        
	{
        //�� ���� �������� �� �� ������������ ������
		if( !((glob.uFixedStep > CROSSING_RANGE1) && (glob.uFixedStep < CROSSING_RANGE2)) )
			glob.uErrorCounter++;
	}
	else
	{
		glob.uErrorCounter = 0;
	}
	SaveAngle(glob.uFixedStep, iSensorAngle, uDiffAngle);
	if(glob.bEnableUART)//����� ��������� ��� ��� ��������� �������� �� ����������, �� ������� ����� �������
	{
		printf("���: %d,\t������: %#X,\t�������: %d\t������ �.: %d,\t������ �.: %d,\t �����������: %d\r\n",
		glob.uFixedStep,glob.StatusIS,glob.uAngleArray[0],iSensorAngle,uCalcAngle,uDiffAngle);
	}
    if(glob.bSetNewStep)
        return;

	if(glob.uErrorCounter >= ERROR_MAXCOUNTER)
	{
		SET_STATUS(glob.StatusIS,ST_SYNC);
		if(glob.bEnableUART)
		{
			printf("\t!!!������ ������������� (���: %d, ����: %d), �����������: %d.\r\n",
													iSensorAngle,uCalcAngle,uDiffAngle);
		}
		PointerReset();
	}
}
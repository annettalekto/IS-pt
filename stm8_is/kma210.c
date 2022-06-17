#include "kma210.h"

inline void x_delay(unsigned int i);
static void BitZero(void); 
static void BitOne(void);
bool WaitLevel(bool bLevel);
static u16 ReadWord(void);
static void ProgramByte(u8 uByte);
static void ProgramStart(void);
static void ProgramStop(void);
static void Handover(void);
static void Takeover(void);
int CalcCRC (int iCrc, u16 uData);
u8 GenCrc(void);

#define DELAY_BUTTON    2   //(сек.)длительность нажатия кнопки установки нуля
#define COEFF_ZA        71  //(65536/921)

USE_GLOBALS;
static volatile u16 g_uAngleValue = 0;

/**
  * @brief Получить угол с датчика, присоединённого к АЦП
  * @par
  * Параметры отсутствуют
  * @retval -1 если ошибка, либо значение с датчика
*/
s16 GetAngle_KMA210(void)
{
  static u32 uZeros = 0xFFFF;
  
  if ((0 == g_uAngleValue) && uZeros--)
    return ANGLE_ERROR;
  else
    return g_uAngleValue;
}

/**
  * @brief Инициализация аналого-цифрового преобразователя
  * @par
  * Параметры отсутствуют
  * @retval 
  * Возвращаемое значение отсутствует
*/
void Init_KMA210(void)
{
  ADC2_DeInit();
  //ADC2_PRESSEL_FCPU_D8 2 3 4 6 8
  ADC2_Init(ADC2_CONVERSIONMODE_CONTINUOUS, ADC2_CHANNEL_12, ADC2_PRESSEL_FCPU_D8,
    ADC2_EXTTRIG_TIM, DISABLE, ADC2_ALIGN_RIGHT, ADC2_SCHMITTTRIG_CHANNEL12, DISABLE);
  ADC2_ITConfig(ENABLE);
  ADC2_StartConversion();  
}

/**
  * @brief Прерывание ADC2
  * @par Parameters:
  * Параметры отсутствуют
  * @retval
  * Возвращаемое значение отсутствует
*/
void ADC2_IRQHandler(void) interrupt 22
{
  g_uAngleValue = ADC2_GetConversionValue();
  ADC2_ClearITPendingBit();
}

//=============================================================================
//                          РЕЖИМ ПРОГРАММИРОВАНИЯ
//=============================================================================

inline void x_delay(unsigned int i) 
{ 
  while (--i) _nop_();
} 

#define delay_1us()     _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
#define delay_5us()     delay_1us(); delay_1us(); delay_1us(); delay_1us(); delay_1us(); delay_1us(); _nop_();_nop_(); _nop_();_nop_();
#define delay_10us()    delay_5us(); delay_5us(); _nop_();_nop_(); _nop_();
#define delay_20us()    delay_10us(); delay_10us(); //0.25*tbit = 20us
#define delay_40us()    delay_20us(); delay_20us();
#define delay_60us()    delay_40us(); delay_20us(); _nop_(); //0.75*tbit = 60us
#define delay_80us()    delay_40us(); delay_40us(); _nop_(); _nop_(); _nop_(); _nop_();//tbit = 80us
#define delay_20ms()    x_delay(8600); x_delay(8600); x_delay(8600); x_delay(8600);//tprog = 20ms
#define delay_25ms()    x_delay(8600); x_delay(8600); x_delay(8600); x_delay(8600); x_delay(8600);

#define delay_10ms()    x_delay(8600); x_delay(8600); x_delay(8600); //tprog = 20ms
#define delay_5ms()    x_delay(8600); x_delay(8600);

static void BitZero(void)
{
  // 0.25  
  WRITE_1(KMA210_PORT, KMA210_PIN);
  delay_20us();
  WRITE_0(KMA210_PORT, KMA210_PIN);
  delay_60us();
}

static void BitOne(void)
{
  // 0.75
  WRITE_1(KMA210_PORT, KMA210_PIN);
  delay_60us();
  WRITE_0(KMA210_PORT, KMA210_PIN);  
  delay_20us();
}

/*static u16 ReadWord(void)
{
  u16 uResult = 0;
  u16 uMask   = 0x8000;
  do
  {
    delay_40us();
    if (IN_KMA210) uResult |= uMask;
    delay_40us();
    uMask = (uMask == 1) ?  0 : uMask >> 1;
  } while (uMask);  
  return uResult;
}*/

bool WaitLevel(bool bLevel)
{
    u8 Count = 16;      // 80 мкс 
    bool bPinLevel;
    bool bOut = false;
    do
    {
        delay_5us();
        bPinLevel = IN_KMA210;
        Count--;
    
        if(bLevel) bOut = !bPinLevel;
        else bOut = bPinLevel;
    
    } while(bOut && Count);
    if(bOut == bLevel) return true;
    return false;
}

static u16 ReadWord(void)
{
  u16 uResult = 0;
  u16 uMask   = 0x8000;

  do
  {
    WaitLevel(true);
    delay_40us();
    if (IN_KMA210)
    {
        uResult |= uMask;
        WaitLevel(false);
    }
    uMask = (uMask == 1) ?  0 : uMask >> 1;
  } while (uMask);  
  return uResult;
}

static void ProgramByte(u8 uByte)
{
  u8 uMask = 0x80;
  do
  {
    if (uMask & uByte) BitOne(); else BitZero();
    uMask >>= 1;
  } while (uMask);
}

static void ProgramStart(void)
{
  WRITE_0(KMA210_PORT, KMA210_PIN);  
  delay_80us();
  WRITE_1(KMA210_PORT, KMA210_PIN);  
}

static void ProgramStop(void)
{
  WRITE_1(KMA210_PORT, KMA210_PIN);  
  delay_80us();
}

static void Handover(void)
{
  // handover bit (logic 0), 3/4 tbit 
  WRITE_1(KMA210_PORT, KMA210_PIN);
  delay_20us();
  WRITE_0(KMA210_PORT, KMA210_PIN);
  delay_40us(); 
  // disable output
  GPIO_Init(KMA210_PORT, KMA210_PIN,GPIO_MODE_IN_PU_NO_IT);
}

static void Takeover(void)
{
  GPIO_Init(KMA210_PORT, KMA210_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  WRITE_0(KMA210_PORT, KMA210_PIN);  
  delay_60us();
}

/*
  COMMAND FORMAT:
  7   6   5   4   3   2   1   0
  RG  RG  RG  A   A   A   A   RW

  RG - region: 000 - 16-bit nonvolatile memory
               100 - 16-bit register
  A  - address bits
  RW - 0 - write, 1 - read
*/

#define READ              1
#define CTRL1             0x82
#define TESTCTRL0         0x96
#define ZERO_ANGLE        0x10
#define ANG_RNG_MULT_MSB  0x12
#define ANG_RNG_MULT_LSB  0x14
#define CLAMP_LO          0x16
#define CLAMP_HI          0x18
#define ID_LO             0x1A
#define ID_HI             0x1C
#define CTRL_CUST         0x1E


/*  ZeroAngle,  AngRngMultMSB,  AngRngMultLSB,  ClampLO,
    ClampHI,    Id_LО,          Id_HI,          Ctrl_Cust */
u16 uDataSeq[] = {   0x00FF, 0xFFC1, 0x0400, 0x0100,
                    0x1300, 0x0000, 0x0000, 0x0000 };
 
int CalcCRC (int iCrc, u16 uData)
{
    int iGpoly = 0x107;
    int i;

    for(i=15; i>=0; i--)
    {
        iCrc <<=1;
        iCrc |= (int) ((uData & (1u << i)) >> i); 
        if(iCrc & 0x100) iCrc ^= iGpoly;
    }
    return iCrc;
}

u8 GenCrc(void)
{
    int iCrc, i;
    iCrc = 0xFF;
    
    for(i=0; i<=7; i++)
    {
        iCrc = CalcCRC(iCrc, uDataSeq[i]);
    }
    return (u8)iCrc;  
}

//запись в регистр ZERO_ANGLE числа uDataSeq[0]
void ProgramKMA210(void)
{
    //u16 Word1, Word2, Word3, Word4, Word5, Word6, Word7, Word8;
    u8 uByteLo = 0;
    u8 uByteHi = 0;
    u8 uCrc;

    uCrc=GenCrc();

    ADC2_DeInit();
    GPIO_Init(KMA210_PORT, KMA210_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    //delay_25ms();

    //питание kma210 в режиме программирования
    GPIO_Init(POWER_KMA210_PORT, POWER_KMA210_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
   // WRITE_0(POWER_KMA210_PORT, POWER_KMA210_PIN);
    delay_25ms();
    delay_25ms();
    delay_25ms();
    WRITE_1(POWER_KMA210_PORT, POWER_KMA210_PIN);
    delay_10ms();//tcmd = 20ms после подачи питания
    //delay_5ms();
    
    // Войти в режим программирования
    ProgramStart();
    ProgramByte(0x94);
    ProgramByte(0x16);
    ProgramByte(0xF4);
    ProgramStop();
    
    //delay_80us();
    //delay_80us();
    //delay_80us();

    //включение записи
    ProgramStart();
    ProgramByte(TESTCTRL0);
    ProgramByte(0xFF);      
    ProgramByte(0xFF);
    ProgramStop();
    //включение charge pump
    ProgramStart();
    ProgramByte(CTRL1);
    ProgramByte(0xFF);
    ProgramByte(0xFF);
    ProgramStop();
    delay_20ms();

    //запись 
    uByteLo = (u8)uDataSeq[0];
    uByteHi = (u8)(uDataSeq[0]>>8);
    ProgramStart();
    ProgramByte(ZERO_ANGLE);
    ProgramByte(uByteHi);
    ProgramByte(uByteLo);
    ProgramStop();
    delay_25ms();//programming time 20ms и более
      
    //запись crc
    ProgramStart();
    ProgramByte(CTRL_CUST);
    ProgramByte(0x00);
    ProgramByte(uCrc);
    ProgramStop();
    delay_25ms();
  
/*  //для отладки
    while(1)
    {      
        //чтение
        Word1 = ReadMemory(ZERO_ANGLE); 
        Word2 = ReadMemory(ANG_RNG_MULT_MSB);
        Word3 = ReadMemory(ANG_RNG_MULT_LSB);
        Word4 = ReadMemory(CLAMP_LO);
        Word5 = ReadMemory(CLAMP_HI);
        Word6 = ReadMemory(ID_LO);
        Word7 = ReadMemory(ID_HI);
        Word8 = ReadMemory(CTRL_CUST);

        delay_25ms();
        delay_25ms();
        delay_25ms();

        //если кнопка нажата
        if (IN_BUTTON)        
        {
            //питание kma210 в режиме программирования
            ResetKMA210
            Init_KMA210();
            break;
        } 
    }
*/
}

u16 ReadMemory(u8 uRegister)
{
    u16 uWord;

    //чтение
    ProgramStart();
    ProgramByte(uRegister|READ);
    Handover();
    uWord = ReadWord();
    Takeover();
    ProgramStop();

    //Init_UART();
    //printf("Register 0x%.2X: 0x%.4X\r\n",uRegister, uWord);
    x_delay(8600);//5ms
    x_delay(8600);

    return uWord;
}

//сброс питания kma210 для перехода рабочий режим 
void ResetKMA210(void)
{
    //питание kma210
    GPIO_Init(POWER_KMA210_PORT, POWER_KMA210_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    WRITE_0(POWER_KMA210_PORT, POWER_KMA210_PIN);
    delay_25ms();
    delay_25ms();
    delay_25ms();
    WRITE_1(POWER_KMA210_PORT, POWER_KMA210_PIN);
    Init_KMA210();
}

bool WriteRegAngle(void)
{       
    bool bResult = true;
    u16 uDataZA = 0;

    //сброс регистра ZA
    uDataSeq[0] = 0x0000;
    ProgramKMA210();
    ResetKMA210();
    delay(0xfff);

    //раcсчитать значение для ZA
    uDataSeq[0] = (MAX_ANGLE_KMA210+5 - g_uAngleValue)*COEFF_ZA;
    //записать ZA
    disableInterrupts();
    ProgramKMA210();
    //считать и проверить
    uDataZA = ReadMemory(ZERO_ANGLE);
    if (uDataSeq[0] != uDataZA)
    {
        bResult = false;
    }    
    ResetKMA210();
    enableInterrupts();
    
	return bResult;  
}

//при установке стрелки ИС на ноль, записать регистр ZERO_ANGLE
bool SetToZero(void)
{
    bool ok = false;
    struct timer tim;
    
    timer_set(&tim, 30*ONE_SECOND);     //ждем 30с пока отпустят
    while(!timer_expired(&tim))
    {
        if(!IN_BUTTON)
        {    
            RESET_TURN();               //сброс стрелки
            ok = true;
            break;
        }
    }
    if(!ok)
        return false;

    ok = false;
    timer_set(&tim, 30*ONE_SECOND);     //ждем 30с пока снова не нажата
    while(!timer_expired(&tim))
    {
        if(IN_BUTTON)
        {
            ok = true;
            break;
        }    
    }
    if(!ok)
        return false;
    
    timer_set(&tim, 2*ONE_SECOND);
    while(!timer_expired(&tim))
    {
        if(!IN_BUTTON)                  //кнопка должна быть нажата 2 сек
            return false;
    }

    ok = WriteRegAngle();
    if(glob.bEnableUART)
    {
        if(ok) printf("Угол записан успешно\r\n");
        else printf("Ошибка записи угла!\r\n");
    }
    
    timer_set(&tim, 2*ONE_SECOND);      //еще 2 сек задержки перед выходом в главный цикл   
    while(!timer_expired( &tim))         //иначе повторно входит в функцию установки нуля
    { ; }

    return ok;
}

void PointerSetToZero(void)
{
    x_delay(0x14FF0);
    x_delay(0x14FF0);

    if(!IN_BUTTON)
        return;
    
    SetToZero();
    InitIS();
}
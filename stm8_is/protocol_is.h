#ifndef _PROTOCOL_IS_H_
#define _PROTOCOL_IS_H_

#include "stm8s_conf.h"
#include "stm8s_exti.h"

/*
    При приеме по Куцак протоколу последовательностей:
    REPROGRAM_CODE_1, REPROGRAM_CODE_2, UART_CODE, ENABLE_CODE/DISABLE_CODE
    - вкл\выкл UART (отладочный режим);
    REPROGRAM_CODE_1, REPROGRAM_CODE_2, BOOTLOADER_CODE, ENABLE_CODE/DISABLE_CODE
    - вкл\выкл bootloader.
    В режиме отладки ИС по Куцак протоколу возвращает погрешность шага.
    Для работы в блоке UART и bootloader должны быть выкл.
*/
#define REPROGRAM_CODE_1    (0x1337 & 0x7FFF)//0x1337
#define REPROGRAM_CODE_2    (0xC0DE & 0x7FFF)//0x40D3
#define UART_CODE           (0x4242 & 0x7FFF)//0x4242
#define BOOTLOADER_CODE     (0xB007 & 0x7FFF)//0x3007
//~ #define ENABLE_CODE         (0x10AD & 0x7FFF)//0x10AD перенесено в signals.h
//~ #define DISABLE_CODE        (0xDEAD & 0x7FFF)//0x5EAD

#define MAX_SPEED_SCALE     760     //150 км/ч, 228 град.
#define MAX_SPEED           780     //макс. кол-во шагов(c докруткой мах) на 234 гр.
#define MAX_STEP            800     //мак. возможный шаг ИС

void InitProtocolIS(void);
bool GetBit(bool *uDataBit);
bool GetByte(void);
bool SendByte(u8 uData);
u8 FormatStatusIS(void);
bool CheckReprogramCommand(u16 uStep);

#endif
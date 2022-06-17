#ifndef _STDINT_H_
#define _STDINT_H_

#include <stm8s_type.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;

#define UINT8_MAX    ((u8)255)
#define INT8_MAX     ((s8)127)
#define INT8_MIN     ((s8)-128)
#define UINT16_MAX   ((u16)65535u)
#define INT16_MAX    ((s16)32767)
#define INT16_MIN    ((s16)-32768)
#define UINT32_MAX   ((u32)4294967295uL)
#define INT32_MAX    ((s32)2147483647)
#define INT32_MIN    ((s32)-2147483648)

#endif //_STDINT_H_

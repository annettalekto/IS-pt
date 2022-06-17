#ifndef _UART3_H_
#define _UART3_H_

#include "stm8s_conf.h"
#include "stm8s_uart1.h"
#include "stm8s_uart3.h"
//~ #include <stdio.h>

//#define CSI "\x1B[" //
//#define CRLF CSI "0K" "\r\n"    //
#define CSI "\x1B[" 
#define CRLF "\r\n" 
#define COLOR_ON printf(CSI "1m" CSI "32m") 
#define COLOR_OFF printf(CSI "0m") 
  
// ÷вет текста в консоли 
#define COLOR_BLACK   printf(CSI "1m" CSI "30m") 
#define COLOR_RED     printf(CSI "1m" CSI "31m") 
#define COLOR_GREEN   printf(CSI "1m" CSI "32m") 
#define COLOR_YELLOW  printf(CSI "1m" CSI "33m") 
#define COLOR_BLUE    printf(CSI "1m" CSI "34m") 
#define COLOR_MAGENTA printf(CSI "1m" CSI "35m") 
#define COLOR_CYAN    printf(CSI "1m" CSI "36m") 
#define COLOR_WHITE   printf(CSI "1m" CSI "37m") 
  
// ÷вет фона в консоли 
#define BACKGROUND_BLACK   printf(CSI "1m" CSI "40m") 
#define BACKGROUND_RED     printf(CSI "1m" CSI "41m") 
#define BACKGROUND_GREEN   printf(CSI "1m" CSI "42m") 
#define BACKGROUND_YELLOW  printf(CSI "1m" CSI "43m") 
#define BACKGROUND_BLUE    printf(CSI "1m" CSI "44m") 
#define BACKGROUND_MAGENTA printf(CSI "1m" CSI "45m") 
#define BACKGROUND_CYAN    printf(CSI "1m" CSI "46m") 
#define BACKGROUND_WHITE   printf(CSI "1m" CSI "47m") 
  
// ќчистить экран и переместить курсор в верхний левый угол экрана 
//#define UART_CLEAR printf("\x1B[2J\x1B[1;1H")
    
// ќчистить экран и переместить курсор в верхний левый угол экрана
//#define UART_CLEANSCREEN      printf(CSI "2J")  //
//#define UART_RETURN           printf(CSI "1;1H")
#define UART_CLEAR printf(CSI "2J" CSI "1;1H")//раб.
#define UART_CLEANSCREENTOEND printf(CSI "0J");

#define SHOW_PORT(x, y, z)    printf("[%c] %s" CRLF, GPIO_ReadInputPin(x,  y)  ? ' ' : '+', z) 
#define SHOW_OUTPORT(x, y, z) printf("[%c] %s" CRLF, GPIO_ReadOutputPin(x,  y) ? ' ' : '§', z)  

#ifdef _RAISONANCE_ 
 #define PUTCHAR_PROTOTYPE int putchar (char c)
#else /* _COSMIC_ */
 #define PUTCHAR_PROTOTYPE char putchar (char c)
#endif /* _RAISONANCE_ */

void Init_UART(void);
void DeInit_UART(void);

#endif
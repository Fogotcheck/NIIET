#ifndef __logger_h__
#define __logger_h__

#include <stdio.h>
#include "K1921VG015.h"

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

#define RETARGET_UART_BAUD 115200
#define SystemCoreClock_uart 16000000
#define RETARGET_UART UART0
#define RETARGET_UART_NUM 0
#define RETARGET_UART_PORT GPIOA
#define RETARGET_UART_PIN_TX_POS 1
#define RETARGET_UART_PIN_RX_POS 0
#define RETARGET_UART_RX_IRQHandler UART0_IRQHandler
#define RETARGET_UART_RX_IRQn UART0_IRQn

#if (DEBUG_LOG > 0)
#define FERROR(...)                                          \
	{                                                    \
		printf("%s:%d ERROR: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__);                         \
		printf("\r\n");                              \
	}
#else
#define FERROR(...)
#endif

#if (DEBUG_LOG > 1)
#define FWARNING(...)                                          \
	{                                                      \
		printf("%s:%d WARNING: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__);                           \
		printf("\r\n");                                \
	}
#else
#define FWARNING(...)
#endif

#if (DEBUG_LOG > 2)
#define FINFO(...)                                          \
	{                                                   \
		printf("%s:%d INFO: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__);                        \
		printf("\r\n");                             \
	}
#else
#define FINFO(...)
#endif

void retarget_init(void);
int __io_putchar(int ch);
int __io_getchar();

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif //__logger_h__

#include "logger.h"

void retarget_init(void)
{
	uint32_t baud_icoef = SystemCoreClock_uart / (16 * RETARGET_UART_BAUD);
	uint32_t baud_fcoef =
		((SystemCoreClock_uart / (16.0f * RETARGET_UART_BAUD) -
		  baud_icoef) *
			 64 +
		 0.5f);
	uint32_t uartclk_ref = 1;

	RCU->CGCFGAHB_bit.GPIOAEN = 1;
	RCU->RSTDISAHB_bit.GPIOAEN = 1;
	RCU->CGCFGAPB_bit.UART0EN = 1;
	RCU->RSTDISAPB_bit.UART0EN = 1;

	RETARGET_UART_PORT->ALTFUNCNUM_bit.PIN0 = 1;
	RETARGET_UART_PORT->ALTFUNCNUM_bit.PIN1 = 1;
	RETARGET_UART_PORT->ALTFUNCSET = (1 << RETARGET_UART_PIN_TX_POS) |
					 (1 << RETARGET_UART_PIN_RX_POS);
	RCU->UARTCLKCFG[RETARGET_UART_NUM].UARTCLKCFG =
		(uartclk_ref << RCU_UARTCLKCFG_CLKSEL_Pos) |
		RCU_UARTCLKCFG_CLKEN_Msk | RCU_UARTCLKCFG_RSTDIS_Msk;
	RETARGET_UART->IBRD = baud_icoef;
	RETARGET_UART->FBRD = baud_fcoef;
	RETARGET_UART->LCRH = UART_LCRH_FEN_Msk | (3 << UART_LCRH_WLEN_Pos);
	RETARGET_UART->CR = UART_CR_TXE_Msk | UART_CR_RXE_Msk |
			    UART_CR_UARTEN_Msk;
}

int __io_putchar(int ch)
{
	while (RETARGET_UART->FR_bit.BUSY) {
	};
	RETARGET_UART->DR = ch;
	return ch;
}

int __io_getchar()
{
	while (RETARGET_UART->FR_bit.RXFE) {
	};
	return (int)RETARGET_UART->DR_bit.DATA;
}

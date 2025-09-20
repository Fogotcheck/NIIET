/*==============================================================================
 * Пример работы DMA с UART1 для К1921ВG015
 * Реализован прием по UART1 16-ти байт через DMA в массив UBUFF
 * После приема происходит отправка содержимого массива UBUFF в UART1 через DMA
 * Настройки UART1:
 * GPIO: TX - A.3,  RX - A.2
 * BaudRate: 115200
 *------------------------------------------------------------------------------
 * НИИЭТ, Александр Дыхно 	<dykhno@niiet.ru>
 * Fogotcheck				<fogotcheck@live.com>
 *==============================================================================
 * ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО
 * ГАРАНТИЙ, ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ
 * ПРИГОДНОСТИ, СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ
 * НАРУШЕНИЙ, НО НЕ ОГРАНИЧИВАЯСЬ ИМИ. ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ
 * ПРЕДНАЗНАЧЕНО ДЛЯ ОЗНАКОМИТЕЛЬНЫХ ЦЕЛЕЙ И НАПРАВЛЕНО ТОЛЬКО НА
 * ПРЕДОСТАВЛЕНИЕ ДОПОЛНИТЕЛЬНОЙ ИНФОРМАЦИИ О ПРОДУКТЕ, С ЦЕЛЬЮ СОХРАНИТЬ ВРЕМЯ
 * ПОТРЕБИТЕЛЮ. НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ
 * ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ, ЗА ПРЯМОЙ ИЛИ КОСВЕННЫЙ УЩЕРБ, ИЛИ
 * ПО ИНЫМ ТРЕБОВАНИЯМ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ
 * ИЛИ ИНЫХ ДЕЙСТВИЙ С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ.
 *
 *                              2025 АО "НИИЭТ"
 *==============================================================================
 */

//-- Includes ------------------------------------------------------------------
#include <K1921VG015.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <system_k1921vg015.h>
#include "retarget.h"

#include "FreeRTOS.h"
#include "task.h"

//-- Defines -------------------------------------------------------------------
#define GPIOA_ALL_Msk 0xFFFF
#define GPIOB_ALL_Msk 0xFFFF

#define LEDS_MSK 0xFF00
#define LED0_MSK (1 << 8)
#define LED1_MSK (1 << 9)
#define LED2_MSK (1 << 10)
#define LED3_MSK (1 << 11)
#define LED4_MSK (1 << 12)
#define LED5_MSK (1 << 13)
#define LED6_MSK (1 << 14)
#define LED7_MSK (1 << 15)

#define UART1_BAUD 115200

#define UBUFF_SIZE 16
uint8_t UBUFF[UBUFF_SIZE]; //Буфер для хранения данных, полученных от UART1
//Объявляем структуру управляющих данных DMA с выравниванием по 1024 байт
DMA_CtrlData_TypeDef DMA_CONFIGDATA __attribute__((aligned(1024)));

void TMR32_IRQHandler();
void DMA_CH_9_IRQHandler();
void DMA_CH_12_IRQHandler();
void MainThr(void *arg);

void BSP_led_init()
{
	//Разрешаем тактирование GPIOA
	RCU->CGCFGAHB_bit.GPIOAEN = 1;
	//Включаем  GPIOA
	RCU->RSTDISAHB_bit.GPIOAEN = 1;
	GPIOA->OUTENSET = LEDS_MSK;
	GPIOA->DATAOUTSET = LEDS_MSK;
}

void TMR32_init(uint32_t period)
{
	RCU->CGCFGAPB_bit.TMR32EN = 1;
	RCU->RSTDISAPB_bit.TMR32EN = 1;

	//Записываем значение периода в CAPCOM[0]
	TMR32->CAPCOM[0].VAL = period - 1;
	//Выбираем режим счета от 0 до значения CAPCOM[0]
	TMR32->CTRL_bit.MODE = 1;

	//Разрешаем прерывание по совпадению значения счетчика и CAPCOM[0]
	TMR32->IM = 2;

	// Настраиваем обработчик прерывания для TMR32
	PLIC_SetIrqHandler(Plic_Mach_Target, IsrVect_IRQ_TMR32,
			   TMR32_IRQHandler);
	PLIC_SetPriority(IsrVect_IRQ_TMR32, 0x1);
	PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_TMR32);
}

//Фукнция настройки базового режима DMA на передачу и прием
void DMA_UART1_init()
{
	// Инцииализация DMA
	// Базовый указатель на первичную структуру управляющих данных
	DMA->BASEPTR = (uint32_t)(&DMA_CONFIGDATA);
	// Инициализация канала на передачу TX (9-й канал DMA)
	/* источник */
	DMA_CONFIGDATA.PRM_DATA.CH[9].SRC_DATA_END_PTR = (uint32_t)&(
		UBUFF[UBUFF_SIZE - 1]); //Адрес конца данных источника
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.SRC_SIZE =
		DMA_CHANNEL_CFG_SRC_SIZE_Byte; //Разрядность данных источника
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.SRC_INC =
		DMA_CHANNEL_CFG_SRC_INC_Byte; // Инкрементируем на байт
	/* приемник */
	DMA_CONFIGDATA.PRM_DATA.CH[9].DST_DATA_END_PTR =
		(uint32_t)(&UART1->DR); //Адрес конца данных приемника
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.DST_SIZE =
		DMA_CHANNEL_CFG_DST_SIZE_Byte; //Разрядность данных приемника
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.DST_INC =
		DMA_CHANNEL_CFG_DST_INC_None; //Не инкрементируем, потому что буфер
	/* общее */
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.R_POWER =
		0x0; // Количество передач до переарбитрации
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.N_MINUS_1 =
		UBUFF_SIZE - 1; //Общее количество передач DMA
	DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.CYCLE_CTRL =
		DMA_CHANNEL_CFG_CYCLE_CTRL_Basic; //Задание типа цикла DMA

	// Инициализация канала на прием RX (12-й канал DMA)
	/* источник */
	DMA_CONFIGDATA.PRM_DATA.CH[12].SRC_DATA_END_PTR =
		(uint32_t)(&UART1->DR); //Адрес источника данных
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.SRC_SIZE =
		DMA_CHANNEL_CFG_SRC_SIZE_Byte; //Разрядность данных источника
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.SRC_INC =
		DMA_CHANNEL_CFG_SRC_INC_None; // Не инкрементируем
	/* приемник */
	DMA_CONFIGDATA.PRM_DATA.CH[12].DST_DATA_END_PTR = (uint32_t)&(
		UBUFF[UBUFF_SIZE - 1]); //Адрес конца данных приемника
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.DST_SIZE =
		DMA_CHANNEL_CFG_DST_SIZE_Byte; //Разрядность данных приемника
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.DST_INC =
		DMA_CHANNEL_CFG_DST_INC_Byte; //Инкрементируем на байт
	/* общее */
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.R_POWER =
		0x0; // Количество передач до переарбитрации
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.N_MINUS_1 =
		UBUFF_SIZE - 1; //Общее количество передач DMA
	DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.CYCLE_CTRL =
		DMA_CHANNEL_CFG_CYCLE_CTRL_Basic; //Задание типа цикла DMA

	// Инциализация контроллера DMA
	DMA->ENSET_bit.CH9 = 1; //Включаем канала DMA 1
	DMA->ENSET_bit.CH12 = 1; //Включаем канала DMA 3
	DMA->CFG_bit.MASTEREN = 1; //Бит разрешения работы контролера DMA
	// PLIC прерывания DMA

	// Настраиваем обработчик прерывания для DMA_CH_9
	PLIC_SetIrqHandler(Plic_Mach_Target, IsrVect_IRQ_DMA3,
			   DMA_CH_9_IRQHandler);
	PLIC_SetPriority(IsrVect_IRQ_DMA3, 0x1);
	PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_DMA3);

	// Настраиваем обработчик прерывания для DMA_CH_12
	PLIC_SetIrqHandler(Plic_Mach_Target, IsrVect_IRQ_DMA4,
			   DMA_CH_12_IRQHandler);
	PLIC_SetPriority(IsrVect_IRQ_DMA4, 0x1);
	PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_DMA4);

	UART1->DMACR_bit.RXDMAE =
		1; //Разрешение DMA обрабатывать запросы приемного буфера UART1
}
void UART1_init()
{
	uint32_t baud_icoef = HSECLK_VAL / (16 * UART1_BAUD);
	uint32_t baud_fcoef =
		((HSECLK_VAL / (16.0f * RETARGET_UART_BAUD) - baud_icoef) * 64 +
		 0.5f);
	// Настраиваем GPIO
	RCU->CGCFGAHB_bit.GPIOAEN = 1;
	RCU->RSTDISAHB_bit.GPIOAEN = 1;
	RCU->CGCFGAPB_bit.UART1EN = 1;
	RCU->RSTDISAPB_bit.UART1EN = 1;

	GPIOA->ALTFUNCNUM_bit.PIN2 = 1;
	GPIOA->ALTFUNCNUM_bit.PIN3 = 1;
	GPIOA->ALTFUNCSET = GPIO_ALTFUNCSET_PIN2_Msk | GPIO_ALTFUNCSET_PIN3_Msk;

	// Настраиваем UART1
	RCU->UARTCLKCFG[1].UARTCLKCFG_bit.CLKSEL = RCU_UARTCLKCFG_CLKSEL_HSE;
	RCU->UARTCLKCFG[1].UARTCLKCFG_bit.DIVEN = 0;
	RCU->UARTCLKCFG[1].UARTCLKCFG_bit.RSTDIS = 1;
	RCU->UARTCLKCFG[1].UARTCLKCFG_bit.CLKEN = 1;

	UART1->IBRD = baud_icoef;
	UART1->FBRD = baud_fcoef;
	UART1->LCRH = UART_LCRH_FEN_Msk | (3 << UART_LCRH_WLEN_Pos);
	UART1->IFLS = 0;
	UART1->CR = UART_CR_TXE_Msk | UART_CR_RXE_Msk | UART_CR_UARTEN_Msk;
}

//-- Peripheral init functions -------------------------------------------------
void periph_init()
{
	BSP_led_init();
	SystemInit();
	SystemCoreClockUpdate();
	BSP_led_init();
	retarget_init();
	UART1_init();
	DMA_UART1_init();
	printf("K1921VG015 SYSCLK = %d MHz\n", (int)(SystemCoreClock / 1E6));
	printf("  UID[0] = 0x%X  UID[1] = 0x%X  UID[2] = 0x%X  UID[3] = 0x%X\n",
	       (unsigned int)PMUSYS->UID[0], (unsigned int)PMUSYS->UID[1],
	       (unsigned int)PMUSYS->UID[2], (unsigned int)PMUSYS->UID[3]);
	printf("  Start UART1(TX - A.3,  RX - A.2) DMA\n");
}

//--- USER FUNCTIONS ----------------------------------------------------------------------
void freertos_risc_v_trap_handler();
volatile uint32_t led_shift;
//-- Main ----------------------------------------------------------------------
int main(void)
{
	InterruptDisable();
	write_csr(mtvec, freertos_risc_v_trap_handler);

	periph_init();
	memset(UBUFF, 0, UBUFF_SIZE); //Очистка буфера UBUFF
	led_shift = LED0_MSK;

	BaseType_t ret = xTaskCreate(MainThr, "MainTask", 256, NULL, 5, NULL);
	if (ret != pdPASS) {
		while (1)
			;
	}

	InterruptEnable();
	vTaskStartScheduler();
	while (1) {
	}

	return 0;
}

void MainThr(__attribute__((unused)) void *arg)
{
	TMR32_init(SystemCoreClock >> 4);
	while (1) {
		vTaskDelay(1000);
	}
}

//-- IRQ INTERRUPT HANDLERS ---------------------------------------------------------------
void TMR32_IRQHandler()
{
	GPIOA->DATAOUTTGL = led_shift;
	led_shift = led_shift << 1;
	if (led_shift > LED7_MSK)
		led_shift = LED0_MSK;
	//Сбрасываем флаг прерывания таймера
	TMR32->IC = 3;
}

void DMA_CH_9_IRQHandler()
{
	// Проверяем статус прерывания DMA от канала №9
	if (DMA->IRQSTAT_bit.CH9) {
		// Сбрасываем статус прерывания DMA от канала №9
		DMA->IRQSTATCLR = DMA_IRQSTATCLR_CH9_Msk;
		UART1->DMACR_bit.TXDMAE =
			0; //Запрет DMA обрабатывать запросы передающего буфера UART1
		memset(UBUFF, 0, UBUFF_SIZE); //Очистка буфера UBUFF
		DMA_UART1_init();
	}
}

void DMA_CH_12_IRQHandler()
{
	uint16_t i;
	// Проверяем статус прерывания DMA от канала №12
	if (DMA->IRQSTAT_bit.CH12) {
		// Сбрасываем статус прерывания DMA от канала №12
		DMA->IRQSTATCLR = DMA_IRQSTATCLR_CH12_Msk;
		UART1->DMACR_bit.RXDMAE =
			0; //Запрет DMA обрабатывать запросы приемного буфера UART1
		printf("\nUART1 Echo: ");
		for (i = 0; i < UBUFF_SIZE; i++)
			retarget_put_char(UBUFF[i]);
		UART1->DMACR_bit.TXDMAE =
			1; //Разрешение DMA обрабатывать запросы передающего буфера UART1
	}
}

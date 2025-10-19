/**
 * @file uart_dma_example.c
 * @brief Example of DMA operation with UART1 for K1921VG015 MCU.
 * 
 * This example implements receiving 16 bytes over UART1 via DMA into the UBUFF array.
 * After receiving, the contents of UBUFF are sent back via UART1 using DMA.
 * The code and description are based on an example from NIIET with added FreeRTOS port.
 * 
 * UART1 settings:
 * - GPIO: TX - A.3, RX - A.2
 * - BaudRate: 115200
 * 
 * @authors
 * - Alexander Dykhno <dykhno@niiet.ru>
 * - Fogotcheck <fogotcheck@live.com>
 * 
 * @note
 * This software is provided "AS IS", without any warranties including merchantability,
 * fitness for a particular purpose or noninfringement. It is provided solely for 
 * informational purposes to save time for users. Authors and copyright holders are not 
 * liable for any claims or damages arising from use or other dealings with the software.
 * 
 * @copyright 2025 AO "NIIET"
 */

/** Includes ------------------------------------------------------------------ */
#include <K1921VG015.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <system_k1921vg015.h>
#include "logger.h"

#include "FreeRTOS.h"
#include "task.h"
#include "freeRTOS_RiscV_provider.h"


/** Defines ------------------------------------------------------------------- */
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

/** @brief Buffer for storing data received from UART1 */
uint8_t UBUFF[UBUFF_SIZE];

/** @brief DMA control data structure aligned on 1024-byte boundary */
DMA_CtrlData_TypeDef DMA_CONFIGDATA __attribute__((aligned(1024)));


/** Function prototypes */
void TMR32_IRQHandler(void);
void DMA_CH_9_IRQHandler(void);
void DMA_CH_12_IRQHandler(void);
void MainThr(void *arg);


/**
 * @brief Initialize LEDs on GPIOA pins.
 *
 * Enables clock for GPIOA and sets LED pins as output with initial high state.
 */
void BSP_led_init()
{
    RCU->CGCFGAHB_bit.GPIOAEN = 1;      /**< Enable GPIOA clock */
    RCU->RSTDISAHB_bit.GPIOAEN = 1;     /**< Release GPIOA reset */
    GPIOA->OUTENSET = LEDS_MSK;         /**< Set LED pins as output */
    GPIOA->DATAOUTSET = LEDS_MSK;       /**< Set LED pins high */
}


/**
 * @brief Initialize 32-bit timer with given period.
 * @param period Timer period in clock ticks
 *
 * Configures TMR32 in count-up mode to specified period with interrupt on match.
 */
void TMR32_init(uint32_t period)
{
    RCU->CGCFGAPB_bit.TMR32EN = 1;      /**< Enable TMR32 clock */
    RCU->RSTDISAPB_bit.TMR32EN = 1;     /**< Release TMR32 reset */

    TMR32->CAPCOM[0].VAL = period - 1;  /**< Set timer period */
    TMR32->CTRL_bit.MODE = 1;           /**< Count up mode */

    TMR32->IM = 2;                      /**< Enable interrupt on CAPCOM[0] match */

    /* Configure interrupt handler and priority in PLIC */
    PLIC_SetIrqHandler(Plic_Mach_Target, IsrVect_IRQ_TMR32, TMR32_IRQHandler);
    PLIC_SetPriority(IsrVect_IRQ_TMR32, 0x1);
    PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_TMR32);
}


/**
 * @brief Initialize DMA channels for UART1 transmit and receive.
 *
 * Sets up DMA channel 9 for UART1 TX and channel 12 for UART1 RX
 * with 16-byte transfers to/from buffer UBUFF.
 */
void DMA_UART1_init()
{
    DMA->BASEPTR = (uint32_t)(&DMA_CONFIGDATA);  /**< Set base pointer for DMA control data */

    // Configure DMA channel 9 (TX)
    DMA_CONFIGDATA.PRM_DATA.CH[9].SRC_DATA_END_PTR = (uint32_t)&(UBUFF[UBUFF_SIZE - 1]);
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.SRC_SIZE = DMA_CHANNEL_CFG_SRC_SIZE_Byte;
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.SRC_INC = DMA_CHANNEL_CFG_SRC_INC_Byte;
    DMA_CONFIGDATA.PRM_DATA.CH[9].DST_DATA_END_PTR = (uint32_t)(&UART1->DR);
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.DST_SIZE = DMA_CHANNEL_CFG_DST_SIZE_Byte;
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.DST_INC = DMA_CHANNEL_CFG_DST_INC_None;
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.R_POWER = 0x0;
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.N_MINUS_1 = UBUFF_SIZE - 1;
    DMA_CONFIGDATA.PRM_DATA.CH[9].CHANNEL_CFG_bit.CYCLE_CTRL = DMA_CHANNEL_CFG_CYCLE_CTRL_Basic;

    // Configure DMA channel 12 (RX)
    DMA_CONFIGDATA.PRM_DATA.CH[12].SRC_DATA_END_PTR = (uint32_t)(&UART1->DR);
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.SRC_SIZE = DMA_CHANNEL_CFG_SRC_SIZE_Byte;
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.SRC_INC = DMA_CHANNEL_CFG_SRC_INC_None;
    DMA_CONFIGDATA.PRM_DATA.CH[12].DST_DATA_END_PTR = (uint32_t)(&UBUFF[UBUFF_SIZE - 1]);
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.DST_SIZE = DMA_CHANNEL_CFG_DST_SIZE_Byte;
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.DST_INC = DMA_CHANNEL_CFG_DST_INC_Byte;
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.R_POWER = 0x0;
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.N_MINUS_1 = UBUFF_SIZE - 1;
    DMA_CONFIGDATA.PRM_DATA.CH[12].CHANNEL_CFG_bit.CYCLE_CTRL = DMA_CHANNEL_CFG_CYCLE_CTRL_Basic;

    DMA->ENSET_bit.CH9 = 1;       /**< Enable DMA channel 9 */
    DMA->ENSET_bit.CH12 = 1;      /**< Enable DMA channel 12 */
    DMA->CFG_bit.MASTEREN = 1;    /**< Enable DMA controller */

    // Configure DMA interrupts in PLIC
    PLIC_SetIrqHandler(Plic_Mach_Target, IsrVect_IRQ_DMA3, DMA_CH_9_IRQHandler);
    PLIC_SetPriority(IsrVect_IRQ_DMA3, 0x1);
    PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_DMA3);

    PLIC_SetIrqHandler(Plic_Mach_Target, IsrVect_IRQ_DMA4, DMA_CH_12_IRQHandler);
    PLIC_SetPriority(IsrVect_IRQ_DMA4, 0x1);
    PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_DMA4);

    UART1->DMACR_bit.RXDMAE = 1; /**< Enable UART1 RX DMA requests */
}


/**
 * @brief Initialize UART1 peripheral.
 *
 * Sets baud rate to 115200, configures GPIO pins A.2 and A.3 for UART,
 * enables UART1 transmitter and receiver.
 */
void UART1_init()
{
    uint32_t baud_icoef = HSECLK_VAL / (16 * UART1_BAUD);
    uint32_t baud_fcoef =
        ((HSECLK_VAL / (16.0f * RETARGET_UART_BAUD) - baud_icoef) * 64 + 0.5f);

    RCU->CGCFGAHB_bit.GPIOAEN = 1;      /**< Enable GPIOA clock */
    RCU->RSTDISAHB_bit.GPIOAEN = 1;     /**< Release GPIOA reset */
    RCU->CGCFGAPB_bit.UART1EN = 1;      /**< Enable UART1 clock */
    RCU->RSTDISAPB_bit.UART1EN = 1;     /**< Release UART1 reset */

    GPIOA->ALTFUNCNUM_bit.PIN2 = 1;     /**< Set alternate function for PA2 (RX) */
    GPIOA->ALTFUNCNUM_bit.PIN3 = 1;     /**< Set alternate function for PA3 (TX) */
    GPIOA->ALTFUNCSET = GPIO_ALTFUNCSET_PIN2_Msk | GPIO_ALTFUNCSET_PIN3_Msk;

    RCU->UARTCLKCFG[1].UARTCLKCFG_bit.CLKSEL = RCU_UARTCLKCFG_CLKSEL_HSE;
    RCU->UARTCLKCFG[1].UARTCLKCFG_bit.DIVEN = 0;
    RCU->UARTCLKCFG[1].UARTCLKCFG_bit.RSTDIS = 1;
    RCU->UARTCLKCFG[1].UARTCLKCFG_bit.CLKEN = 1;

    UART1->IBRD = baud_icoef;           /**< Integer baud rate divisor */
    UART1->FBRD = baud_fcoef;           /**< Fractional baud rate divisor */
    UART1->LCRH = UART_LCRH_FEN_Msk | (3 << UART_LCRH_WLEN_Pos); /**< Enable FIFOs, 8-bit data */
    UART1->IFLS = 0;                    /**< Interrupt FIFO level select */
    UART1->CR = UART_CR_TXE_Msk | UART_CR_RXE_Msk | UART_CR_UARTEN_Msk; /**< Enable TX, RX, UART */
}


/**
 * @brief Initialize peripherals and system.
 *
 * Calls LED init, system setup, UART and DMA init,
 * and outputs system info via logging.
 */
void periph_init()
{
    BSP_led_init();
    SystemInit();
    SystemCoreClockUpdate();
    BSP_led_init();
    retarget_init();
    UART1_init();
    DMA_UART1_init();
    FINFO("K1921VG015 SYSCLK = %d MHz", (int)(SystemCoreClock / 1E6));
    FINFO("UID[0] = 0x%X  UID[1] = 0x%X  UID[2] = 0x%X  UID[3] = 0x%X",
          (unsigned int)PMUSYS->UID[0], (unsigned int)PMUSYS->UID[1],
          (unsigned int)PMUSYS->UID[2], (unsigned int)PMUSYS->UID[3]);
    FINFO("Start UART1(TX - A.3,  RX - A.2) DMA\r\n");
}


/** Global variable to track LED shift pattern */
volatile uint32_t led_shift;


/**
 * @brief Application entry point.
 *
 * Disables interrupts, initializes FreeRTOS, peripherals,
 * creates main task, enables interrupts, and starts scheduler.
 *
 * @return Integer status (never returns under normal operation).
 */
int main(void)
{
    InterruptDisable();
    freertos_risc_v_provider_init();

    periph_init();
    memset(UBUFF, 0, UBUFF_SIZE); /**< Clear UBUFF buffer */
    led_shift = LED0_MSK;

    BaseType_t ret = xTaskCreate(MainThr, "MainTask", 256, NULL, 5, NULL);
    if (ret != pdPASS) {
        while (1)
            ; /**< Error: task creation failed, infinitely wait */
    }

    InterruptEnable();
    vTaskStartScheduler();

    while (1) {
        ; /**< Infinite loop after scheduler start (should never be reached) */
    }

    return 0;
}


/**
 * @brief Main task executed by FreeRTOS.
 *
 * Initializes timer and outputs example log messages,
 * then delays in a loop.
 *
 * @param arg Unused argument pointer.
 */
void MainThr(__attribute__((unused)) void *arg)
{
    TMR32_init(SystemCoreClock >> 4);

    FWARNING("\texample::\t%f", 0.123);
    FERROR("\t\texample::\t%f", 0.123);
    FINFO("\t\texample::\t%s", "Hello world");

    while (1) {
        vTaskDelay(1000); /**< Delay 1000ms */
    }
}


/**
 * @brief Timer32 interrupt handler.
 *
 * Toggles LEDs by shifting the pattern, loops back to first LED.
 * Clears interrupt flag to acknowledge the timer interrupt.
 */
void TMR32_IRQHandler()
{
    GPIOA->DATAOUTTGL = led_shift;
    led_shift = led_shift << 1;
    if (led_shift > LED7_MSK)
        led_shift = LED0_MSK;

    TMR32->IC = 3; /**< Clear timer interrupt flag */
}


/**
 * @brief DMA channel 9 interrupt handler (TX channel).
 *
 * Handles DMA transfer completion for UART1 TX.
 * Disables TX DMA requests, clears buffer, and reinitializes DMA.
 */
void DMA_CH_9_IRQHandler()
{
    if (DMA->IRQSTAT_bit.CH9) {
        DMA->IRQSTATCLR = DMA_IRQSTATCLR_CH9_Msk; /**< Clear DMA channel 9 interrupt */
        UART1->DMACR_bit.TXDMAE = 0;               /**< Disable TX DMA */
        memset(UBUFF, 0, UBUFF_SIZE);               /**< Clear UBUFF buffer */
        DMA_UART1_init();                           /**< Reinitialize DMA */
    }
}


/**
 * @brief DMA channel 12 interrupt handler (RX channel).
 *
 * Handles DMA transfer completion for UART1 RX.
 * Disables RX DMA requests, outputs received data via printf,
 * then enables TX DMA requests for echo.
 */
void DMA_CH_12_IRQHandler()
{
    uint16_t i;

    if (DMA->IRQSTAT_bit.CH12) {
        DMA->IRQSTATCLR = DMA_IRQSTATCLR_CH12_Msk; /**< Clear DMA channel 12 interrupt */
        UART1->DMACR_bit.RXDMAE = 0;               /**< Disable RX DMA */
        FINFO("\nUART1 Echo: ");
        for (i = 0; i < UBUFF_SIZE; i++)
            __io_putchar(UBUFF[i]);                /**< Output received data */
        UART1->DMACR_bit.TXDMAE = 1;               /**< Enable TX DMA to echo data */
    }
}

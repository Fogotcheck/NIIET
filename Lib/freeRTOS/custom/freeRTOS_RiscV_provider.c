#include "freeRTOS_RiscV_provider.h"
#include "plic.h"
#include <system_k1921vg015.h>

void freertos_risc_v_trap_handler();

/**
 * @brief Initialize the FreeRTOS RISC-V provider.
 *
 * This function sets the machine trap-vector base-address
 * register (mtvec) to point to the FreeRTOS trap handler.
 * By doing so, all traps (interrupts and exceptions) will be
 * vectored to the FreeRTOS trap handler routine.
 */
void freertos_risc_v_provider_init(void)
{
	write_csr(mtvec, freertos_risc_v_trap_handler);
}

/**
 * @brief FreeRTOS RISC-V application-specific exception handler.
 *
 * This handler is called by the low-level trap handler when
 * an exception occurs that is not otherwise handled.
 * It forwards execution to the common FreeRTOS trap handler.
 */
void freertos_risc_v_application_exception_handler(void)
{
	trap_handler();
}

/**
 * @brief FreeRTOS RISC-V application-specific interrupt handler.
 *
 * This handler is invoked by the low-level trap handler when
 * an interrupt occurs. It forwards processing to the common
 * FreeRTOS trap handler for further handling.
 */
void freertos_risc_v_application_interrupt_handler(void)
{
	trap_handler();
}

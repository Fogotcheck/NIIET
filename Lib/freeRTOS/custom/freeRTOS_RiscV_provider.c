#include "FreeRTOS.h"
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

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/* External Idle and Timer task static memory allocation functions */
extern void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
					  StackType_t **ppxIdleTaskStackBuffer,
					  uint32_t *pulIdleTaskStackSize);
extern void
vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
			       StackType_t **ppxTimerTaskStackBuffer,
			       uint32_t *pulTimerTaskStackSize);

/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
__attribute__((weak)) void
vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
			      StackType_t **ppxIdleTaskStackBuffer,
			      uint32_t *pulIdleTaskStackSize)
{
	/* Idle task control block and stack */
	static StaticTask_t Idle_TCB;
	static StackType_t Idle_Stack[configMINIMAL_STACK_SIZE];

	*ppxIdleTaskTCBBuffer = &Idle_TCB;
	*ppxIdleTaskStackBuffer = &Idle_Stack[0];
	*pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}

/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
__attribute__((weak)) void
vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
			       StackType_t **ppxTimerTaskStackBuffer,
			       uint32_t *pulTimerTaskStackSize)
{
	/* Timer task control block and stack */
	static StaticTask_t Timer_TCB;
	static StackType_t Timer_Stack[configTIMER_TASK_STACK_DEPTH];

	*ppxTimerTaskTCBBuffer = &Timer_TCB;
	*ppxTimerTaskStackBuffer = &Timer_Stack[0];
	*pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}
#endif

/**
  Dummy implementation of the callback function vApplicationIdleHook().
*/
#if (configUSE_IDLE_HOOK == 1)
__attribute__((weak)) void vApplicationIdleHook(void)
{
	__asm volatile("wfi");
}
#endif
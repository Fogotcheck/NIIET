#ifndef __freeRTOS_RiscV_provider_h__
#define __freeRTOS_RiscV_provider_h__

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @brief Initialize the FreeRTOS RISC-V provider.
 *
 * This function sets the machine trap-vector base-address
 * register (mtvec) to point to the FreeRTOS trap handler.
 * By doing so, all traps (interrupts and exceptions) will be
 * vectored to the FreeRTOS trap handler routine.
 */
void freertos_risc_v_provider_init(void);

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif //__freeRTOS_RiscV_provider_h__

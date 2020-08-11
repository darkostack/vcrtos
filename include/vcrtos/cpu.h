#ifndef VCRTOS_CPU_H
#define VCRTOS_CPU_H

#include <stdint.h>

#include <vcrtos/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This marker is used to identify the stacks beginning */
#define STACK_MARKER (0x77777777)

/* Initial program status register value for a newly created thread */
#define INITIAL_XPSR (0x01000000)

/* ARM Cortex-M specific exception return value, that triggers the return to the
 * task mode stack pointer */
#define EXCEPT_RET_TASK_MODE (0xfffffffd)

/* Interrupt stack canary value 0xe7fe is the ARM Thumb machine code equivalent
 * of asm("bl #-2\n") or 'while (1);', i.e. an infinite loop */
#define STACK_CANARY_WORD (0xE7FEE7FEu)

unsigned cpu_irq_disable(void);

unsigned cpu_irq_enable(void);

void cpu_irq_restore(unsigned state);

int cpu_is_in_isr(void);

void cpu_end_of_isr(instance_t *instance);

void cpu_trigger_pendsv_interrupt(void);

void cpu_switch_context_exit(void);

void cpu_print_last_instruction(void);

void cpu_sleep_until_event(void);

void cpu_sleep(int deep);

void cpu_jump_to_image(uint32_t image_addr);

uint32_t cpu_get_image_base_addr(void);

void *cpu_get_msp(void);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_CPU_H */

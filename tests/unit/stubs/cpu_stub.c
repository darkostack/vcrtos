#include "test-helper.h"

static int is_cpu_in_isr = 0;
static int is_pendsv_interrupt_triggered = 0;

void test_helper_set_cpu_in_isr(void)
{
    is_cpu_in_isr = 1;
}

void test_helper_reset_cpu_in_isr(void)
{
    is_cpu_in_isr = 0;
}

int test_helper_is_pendsv_interrupt_triggered(void)
{
    return is_pendsv_interrupt_triggered;
}

void test_helper_reset_pendsv_trigger(void)
{
    is_pendsv_interrupt_triggered = 0;
}

unsigned cpu_irq_disable(void)
{
    return 0;
}

unsigned cpu_irq_enable(void)
{
    return 0;
}

void cpu_irq_restore(unsigned state)
{
    (void) state;
}

int cpu_is_in_isr(void)
{
    return is_cpu_in_isr;
}

void cpu_trigger_pendsv_interrupt(void)
{
    is_pendsv_interrupt_triggered = 1;
}

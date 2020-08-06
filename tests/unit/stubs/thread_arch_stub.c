#include "test-helper.h"

#include <vcrtos/thread.h>

char *thread_arch_stack_init(thread_handler_func_t func, void *arg, void *stack_start, int stack_size)
{
    (void)func;
    (void)arg;
    (void)stack_size;
    return (char*)stack_start;
}

void thread_arch_stack_print(void)
{

}

int thread_arch_stack_usage(void)
{
    return 0;
}

void *thread_arch_isr_stack_pointer(void)
{
    return NULL;
}

void *thread_arch_stack_start(void)
{
    return NULL;
}

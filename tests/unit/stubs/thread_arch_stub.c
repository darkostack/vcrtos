/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

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

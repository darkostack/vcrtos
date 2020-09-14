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

#ifndef VCRTOS_THREAD_H
#define VCRTOS_THREAD_H

#include <stdlib.h>

#include <vcrtos/config.h>
#include <vcrtos/kernel.h>
#include <vcrtos/cib.h>
#include <vcrtos/clist.h>
#include <vcrtos/msg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*thread_handler_func_t)(void *arg);

typedef enum
{
    THREAD_STATUS_STOPPED,
    THREAD_STATUS_SLEEPING,
    THREAD_STATUS_MUTEX_BLOCKED,
    THREAD_STATUS_RECEIVE_BLOCKED,
    THREAD_STATUS_SEND_BLOCKED,
    THREAD_STATUS_REPLY_BLOCKED,
    THREAD_STATUS_FLAG_BLOCKED_ANY,
    THREAD_STATUS_FLAG_BLOCKED_ALL,
    THREAD_STATUS_MBOX_BLOCKED,
    THREAD_STATUS_COND_BLOCKED,
    THREAD_STATUS_RUNNING,
    THREAD_STATUS_PENDING,
    THREAD_STATUS_NUMOF
} thread_status_t;

#define THREAD_STATUS_NOT_FOUND ((thread_status_t)-1)

#if VCRTOS_CONFIG_THREAD_FLAGS_ENABLE
typedef uint16_t thread_flags_t;
#endif

typedef struct thread
{
    char *stack_pointer;
    thread_status_t status;
    uint8_t priority;
    kernel_pid_t pid;
#if VCRTOS_CONFIG_THREAD_FLAGS_ENABLE
    thread_flags_t flags;
    thread_flags_t waited_flags;
#endif
    list_node_t runqueue_entry;
    void *wait_data;
    list_node_t msg_waiters;
    cib_t msg_queue;
    msg_t *msg_array;
    char *stack_start;
    const char *name;
    int stack_size;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} thread_t;

#define THREAD_FLAGS_CREATE_SLEEPING (0x1)
#define THREAD_FLAGS_CREATE_WOUT_YIELD (0x2)
#define THREAD_FLAGS_CREATE_STACKMARKER (0x4)

kernel_pid_t thread_create(void *instance,
                           char *stack,
                           int size,
                           char priority,
                           int flags,
                           thread_handler_func_t func,
                           void *arg,
                           const char *name);

int thread_scheduler_get_context_switch_request(void *instance);

void thread_scheduler_set_context_switch_request(void *instance, unsigned state);

void thread_scheduler_run(void *instance);

void thread_scheduler_set_status(void *instance, thread_t *thread, thread_status_t status);

void thread_scheduler_switch(void *instance, uint8_t priority);

void thread_exit(void *instance);

int thread_pid_is_valid(kernel_pid_t pid);

void thread_yield(void *instance);

thread_t *thread_current(void *instance);

void thread_sleep(void *instance);

int thread_wakeup(void *instance, kernel_pid_t pid);

kernel_pid_t thread_current_pid(void *instance);

thread_t *thread_get_from_scheduler(void *instance, kernel_pid_t pid);

uint64_t thread_get_runtime_ticks(void *instance, kernel_pid_t pid);

const char *thread_status_to_string(thread_status_t status);

uintptr_t thread_measure_stack_free(char *stack);

unsigned thread_get_schedules_stat(void *instance, kernel_pid_t pid);

char *thread_arch_stack_init(thread_handler_func_t func, void *arg, void *stack_start, int size);

void thread_arch_stack_print(void);

int thread_arch_isr_stack_usage(void);

void *thread_arch_isr_stack_pointer(void);

void *thread_arch_isr_stack_start(void);

void thread_arch_yield_higher(void);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_THREAD_H */

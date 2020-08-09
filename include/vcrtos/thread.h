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

void thread_scheduler_run(void *instance);

void thread_exit(void *instance);

int thread_pid_is_valid(kernel_pid_t pid);

void thread_yield(void *instance);

thread_t *thread_current(void *instance);

kernel_pid_t thread_current_pid(void *instance);

char *thread_arch_stack_init(thread_handler_func_t func,
                             void *arg,
                             void *stack_start,
                             int size);

void thread_arch_stack_print(void);

int thread_arch_stack_usage(void);

void *thread_arch_isr_stack_pointer(void);

void *thread_arch_stack_start(void);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_THREAD_H */

#ifndef VCRTOS_MSG_H
#define VCRTOS_MSG_H

#include <stdint.h>

#include <vcrtos/config.h>
#include <vcrtos/kernel.h>
#include <vcrtos/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msg
{
    kernel_pid_t sender_pid;
    uint16_t type;
    union
    {
        void *ptr;
        uint32_t value;
    } content;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} msg_t;

void msg_init(void *instance, msg_t *msg);

int msg_receive(msg_t *msg);

int msg_send(msg_t *msg, kernel_pid_t pid);

int msg_try_send(msg_t *msg, kernel_pid_t pid);

int msg_send_receive(msg_t *msg, msg_t *reply, kernel_pid_t pid);

int msg_reply(msg_t *msg, msg_t *reply);

void msg_active_thread_queue_print(void *instance);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_MSG_H */

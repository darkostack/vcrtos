#ifndef VCRTOS_EVENT_H
#define VCRTOS_EVENT_H

#include <vcrtos/config.h>
#include <vcrtos/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_FLAG_EVENT (0x1)

typedef struct event event_t;

typedef void (*event_handler_func_t)(event_t *event);

struct event
{
    clist_node_t list_node;
    event_handler_func_t handler;
};

typedef struct
{
    clist_node_t event_list;
    thread_t *waiter;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} event_queue_t;

void event_queue_init(void *instance, event_queue_t *queue);

void event_queue_claim(event_queue_t *queue);

void event_post(event_queue_t *queue, event_t *event);

void event_cancel(event_queue_t *queue, event_t *event);

event_t *event_get(event_queue_t *queue);

event_t *event_wait(event_queue_t *queue);

void event_loop(event_queue_t *queue);

void auto_init_event_thread(void);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_EVENT_H */

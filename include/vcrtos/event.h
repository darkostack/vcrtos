#ifndef VCRTOS_EVENT_H
#define VCRTOS_EVENT_H

#include <vcrtos/config.h>
#include <vcrtos/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_FLAG_EVENT (0x1)

typedef struct
{
    clist_node_t list_node;
} event_t;

typedef struct
{
    clist_node_t event_list;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} event_queue_t;

void event_init(event_t *event);

void event_queue_init(void *instance, event_queue_t *queue);

void event_post(event_queue_t *queue, event_t *event, thread_t *thread);

void event_cancel(event_queue_t *queue, event_t *event);

event_t *event_get(event_queue_t *queue);

event_t *event_wait(event_queue_t *queue);

void event_loop(event_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_EVENT_H */

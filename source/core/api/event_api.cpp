#include <vcrtos/config.h>
#include <vcrtos/event.h>
#include <vcrtos/thread.h>

#include "core/code_utils.hpp"
#include "core/instance.hpp"
#include "core/new.hpp"
#include "core/thread.hpp"

#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE

using namespace vc;

void event_queue_init(void *instances, event_queue_t *queue)
{
    Instance &instance = *static_cast<Instance *>(instances);
    queue = new (queue) EventQueue(instance);
}

void event_queue_claim(event_queue_t *queue)
{
    EventQueue &event_queue = *static_cast<EventQueue *>(queue);
    event_queue.claim();
}

void event_post(event_queue_t *queue, event_t *event)
{
    EventQueue &event_queue = *static_cast<EventQueue *>(queue);
    event_queue.event_post(reinterpret_cast<Event *>(event));
}

void event_cancel(event_queue_t *queue, event_t *event)
{
    EventQueue &event_queue = *static_cast<EventQueue *>(queue);
    event_queue.event_cancel(reinterpret_cast<Event *>(event));
}

event_t *event_get(event_queue_t *queue)
{
    EventQueue &event_queue = *static_cast<EventQueue *>(queue);
    return event_queue.event_get();
}

event_t *event_wait(event_queue_t *queue)
{
    EventQueue &event_queue = *static_cast<EventQueue *>(queue);
    return event_queue.event_wait();
}

void event_loop(event_queue_t *queue)
{
    EventQueue &event_queue = *static_cast<EventQueue *>(queue);
    event_queue.event_loop();
}

#endif // #if VCRTOS_CONFIG_THREAD_EVENT_ENABLE

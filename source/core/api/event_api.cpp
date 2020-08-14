#include <vcrtos/event.h>
#include <vcrtos/thread.h>

#include "core/code_utils.hpp"
#include "core/instance.hpp"
#include "core/new.hpp"
#include "core/thread.hpp"

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

#ifndef UNITTEST

extern "C" static void *_thread_event_handler(void *arg)
{
    EventQueue *queue = static_cast<EventQueue *>(arg);

    queue->claim();
    queue->event_loop();

    return NULL;
}

event_queue_t event_queue_highest;
static char _event_queue_highest_stack[VCRTOS_CONFIG_THREAD_EVENT_HIGHEST_STACK_SIZE];

event_queue_t event_queue_medium;
static char _event_queue_medium_stack[VCRTOS_CONFIG_THREAD_EVENT_MEDIUM_STACK_SIZE];

event_queue_t event_queue_lowest;
static char _event_queue_lowest_stack[VCRTOS_CONFIG_THREAD_EVENT_LOWEST_STACK_SIZE];

typedef struct
{
    event_queue_t *queue;
    char *stack;
    size_t stack_size;
    unsigned priority;
} event_threads_t;

const event_threads_t _event_threads[] = {
    { &event_queue_highest, _event_queue_highest_stack, sizeof(_event_queue_highest_stack),
      VCRTOS_CONFIG_THREAD_EVENT_HIGHEST_PRIORITY },
    { &event_queue_medium, _event_queue_medium_stack, sizeof(_event_queue_medium_stack),
      VCRTOS_CONFIG_THREAD_EVENT_MEDIUM_PRIORITY },
    { &event_queue_lowest, _event_queue_lowest_stack, sizeof(_event_queue_lowest_stack),
      VCRTOS_CONFIG_THREAD_EVENT_LOWEST_PRIORITY }
};

void event_thread_init(void *instance, event_queue_t *queue, char *stack, size_t stack_size, unsigned priority)
{
    event_queue_init(instance, queue);

    (void) thread_create(instance, stack, stack_size, priority, 0, _thread_event_handler, queue, "event");
}

void auto_init_event_thread(void *instance)
{
    for (unsigned i = 0; i < ARRAY_LENGTH(_event_threads); i++)
    {
        event_thread_init(instance, _event_threads[i].queue,
                _event_threads[i].stack, _event_threads[i].stack_size,
                _event_threads[i].priority);
    }
}

#endif // #ifndef UNITTEST

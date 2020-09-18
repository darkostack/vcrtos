#include <vcrtos/config.h>
#include <vcrtos/assert.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/thread.h>
#include <vcrtos/mutex.h>

typedef struct
{
    mutex_t *mutex;
    thread_t *thread;
    volatile uint8_t dequeued;
    volatile uint8_t blocking;
    void *instance;
} mutex_thread_t;

static list_node_t *_list_remove(list_node_t *list, list_node_t *node)
{
    while (list->next)
    {
        if (list->next == node)
        {
            list->next = node->next;
            return node;
        }
        list = list->next;
    }

    return list->next;
}

static void _mutex_remove_thread_from_waiting_queue(void *instance,
                                                    mutex_t *mutex,
                                                    thread_t *thread,
                                                    volatile uint8_t *unlocked)
{
    unsigned irqstate = cpu_irq_disable();

    vcassert(mutex != NULL && thread != NULL);

    if (mutex->queue.next != MUTEX_LOCKED && mutex->queue.next != NULL)
    {
        list_node_t *node = _list_remove(&mutex->queue, (list_node_t *)&thread->runqueue_entry);

        /* if thread was removed from the list */
        if (node != NULL)
        {
            if (mutex->queue.next == NULL)
            {
                mutex->queue.next = MUTEX_LOCKED;
            }
            *unlocked = 1;
            thread_scheduler_set_status(instance, thread, THREAD_STATUS_PENDING);
            cpu_irq_restore(irqstate);
            thread_scheduler_switch(instance, thread->priority);
            return;
        }
    }
    *unlocked = 0;
    cpu_irq_restore(irqstate);
}

static void _mutex_timeout(void *arg)
{
    unsigned int irqstate = cpu_irq_disable();
    mutex_thread_t *mt = (mutex_thread_t *)arg;
    mt->blocking = 0;
    _mutex_remove_thread_from_waiting_queue(mt->instance, mt->mutex, mt->thread, &mt->dequeued);
    cpu_irq_restore(irqstate);
}

int mutex_lock_timeout(void *instance, mutex_t *mutex, uint32_t timeout)
{
    ztimer_t t;

    thread_t *curr = thread_current(instance);

    mutex_thread_t mt = { mutex, curr, .dequeued=0, .blocking=1, instance };

    if (timeout != 0)
    {
        t.callback = _mutex_timeout;
        t.arg = (void *)((mutex_thread_t *)&mt);
        ztimer_set(ZTIMER_USEC, &t, timeout);
    }

    if (mt.blocking)
    {
        mutex_lock(mutex);
    }
    else
    {
        int ret = mutex_try_lock(mutex);
        if (ret == 0)
        {
            return -1;
        }
    }

    ztimer_remove(ZTIMER_USEC, &t);
    return -mt.dequeued;
}

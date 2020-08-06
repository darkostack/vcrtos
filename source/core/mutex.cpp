#include "core/instance.hpp"
#include "core/mutex.hpp"
#include "core/thread.hpp"

namespace vc {

int Mutex::set_lock(int blocking)
{
    unsigned state = cpu_irq_disable();

    if (queue.next == NULL)
    {
        /* mutex was unlocked */
        queue.next = MUTEX_LOCKED;

        cpu_irq_restore(state);

        return 1;
    }
    else if (blocking)
    {
        Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

        get<ThreadScheduler>().set_thread_status(current_thread, THREAD_STATUS_MUTEX_BLOCKED);

        if (queue.next == MUTEX_LOCKED)
        {
            queue.next = current_thread->get_runqueue_entry();
            queue.next->next = NULL;
        }
        else
        {
            current_thread->add_to_list(static_cast<List *>(&queue));
        }

        cpu_irq_restore(state);

        ThreadScheduler::yield_higher_priority_thread();

        return 1;
    }
    else
    {
        cpu_irq_restore(state);
        return 0;
    }
}

void Mutex::unlock(void)
{
    unsigned state = cpu_irq_disable();

    if (queue.next == NULL)
    {
        /* mutex was unlocked */
        cpu_irq_restore(state);
        return;
    }

    if (queue.next == MUTEX_LOCKED)
    {
        queue.next = NULL;
        /* mutex was locked but no thread was waiting for it */
        cpu_irq_restore(state);
        return;
    }

    List *next = (static_cast<List *>(&queue))->remove_head();

    Thread *thread = Thread::get_thread_pointer_from_list_member(next);

    get<ThreadScheduler>().set_thread_status(thread, THREAD_STATUS_PENDING);

    if (!queue.next)
    {
        queue.next = MUTEX_LOCKED;
    }

    uint8_t thread_priority = thread->get_priority();

    cpu_irq_restore(state);

    get<ThreadScheduler>().context_switch(thread_priority);
}

void Mutex::unlock_and_sleeping_current_thread(void)
{
    unsigned state = cpu_irq_disable();

    if (queue.next)
    {
        if (queue.next == MUTEX_LOCKED)
        {
            queue.next = NULL;
        }
        else
        {
            List *next = (static_cast<List *>(&queue))->remove_head();

            Thread *thread = Thread::get_thread_pointer_from_list_member(next);

            get<ThreadScheduler>().set_thread_status(thread, THREAD_STATUS_PENDING);

            if (!queue.next)
            {
                queue.next = MUTEX_LOCKED;
            }
        }
    }

    cpu_irq_restore(state);

    get<ThreadScheduler>().sleeping_current_thread();
}

template <> inline Instance &Mutex::get(void) const
{
    return get_instance();
}

template <typename Type> inline Type &Mutex::get(void) const
{
    return get_instance().get<Type>();
}

} // namespace vc

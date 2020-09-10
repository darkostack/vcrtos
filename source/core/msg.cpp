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

#include "core/instance.hpp"
#include "core/msg.hpp"

namespace vc {

int Msg::send(kernel_pid_t target_pid, int blocking, unsigned state)
{
    if (!Thread::is_pid_valid(target_pid))
    {
        //TODO: warning pid it not valid
    }

    Thread *target_thread = get<ThreadScheduler>().get_thread_from_scheduler(target_pid);

    sender_pid = get<ThreadScheduler>().get_current_active_pid();

    if (target_thread == NULL)
    {
        cpu_irq_restore(state);
        return -1;
    }

    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    if (target_thread->get_status() != THREAD_STATUS_RECEIVE_BLOCKED)
    {
        if (target_thread->queued_msg(this))
        {
            cpu_irq_restore(state);

            if (current_thread->get_status() == THREAD_STATUS_REPLY_BLOCKED)
            {
                ThreadScheduler::yield_higher_priority_thread();
            }

            return 1;
        }

        if (!blocking)
        {
            cpu_irq_restore(state);
            return 0;
        }

        current_thread->wait_data = static_cast<void *>(this);

        thread_status_t new_status;

        if (current_thread->get_status() == THREAD_STATUS_REPLY_BLOCKED)
        {
            new_status = THREAD_STATUS_REPLY_BLOCKED;
        }
        else
        {
            new_status = THREAD_STATUS_SEND_BLOCKED;
        }

        get<ThreadScheduler>().set_thread_status(current_thread, new_status);

        current_thread->add_to_list(static_cast<List *>(&target_thread->msg_waiters));

        cpu_irq_restore(state);

        ThreadScheduler::yield_higher_priority_thread();
    }
    else
    {
        Msg *target_msg = static_cast<Msg *>(target_thread->wait_data);

        *target_msg = *this;

        get<ThreadScheduler>().set_thread_status(target_thread, THREAD_STATUS_PENDING);

        cpu_irq_restore(state);

        ThreadScheduler::yield_higher_priority_thread();
    }

    return 1;
}

int Msg::receive(int blocking)
{
    unsigned state = cpu_irq_disable();

    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    int queue_index = -1;

    if (current_thread->msg_array != NULL)
    {
        queue_index = (static_cast<Cib *>(&current_thread->msg_queue))->get();
    }

    if (!blocking && (!current_thread->msg_waiters.next && queue_index == -1))
    {
        cpu_irq_restore(state);
        return -1;
    }

    if (queue_index >= 0)
    {
        *this = *static_cast<Msg *>(&current_thread->msg_array[queue_index]);
    }
    else
    {
        current_thread->wait_data = static_cast<void *>(this);
    }

    List *next = (static_cast<List *>(&current_thread->msg_waiters))->remove_head();

    if (next == NULL)
    {
        if (queue_index < 0)
        {
            get<ThreadScheduler>().set_thread_status(current_thread, THREAD_STATUS_RECEIVE_BLOCKED);

            cpu_irq_restore(state);

            ThreadScheduler::yield_higher_priority_thread();

#ifndef UNITTEST
            vcassert(get<ThreadScheduler>().get_current_active_thread()->get_status() != THREAD_STATUS_RECEIVE_BLOCKED);
#endif
        }
        else
        {
            cpu_irq_restore(state);
        }

        return 1;
    }
    else
    {
        Thread *sender_thread = Thread::get_thread_pointer_from_list_member(next);

        Msg *tmp = NULL;

        if (queue_index >= 0)
        {
            /* We've already got a message from the queue. As there is a
             * waiter, take it's message into the just freed queue space. */
            tmp = static_cast<Msg *>(&current_thread->msg_array[(static_cast<Cib *>(&current_thread->msg_queue))->put()]);
        }

        /* copy msg */
        Msg *sender_msg = static_cast<Msg *>(sender_thread->wait_data);

        if (tmp != NULL)
        {
            *tmp = *sender_msg;
            *this = *tmp;
        }
        else
        {
            *this = *sender_msg;
        }

        /* remove sender from queue */
        uint8_t sender_priority = KERNEL_THREAD_PRIORITY_IDLE;

        if (sender_thread->get_status() != THREAD_STATUS_REPLY_BLOCKED)
        {
            sender_thread->wait_data = NULL;

            get<ThreadScheduler>().set_thread_status(sender_thread, THREAD_STATUS_PENDING);

            sender_priority = sender_thread->get_priority();
        }

        cpu_irq_restore(state);

        if (sender_priority < KERNEL_THREAD_PRIORITY_IDLE)
        {
            get<ThreadScheduler>().context_switch(sender_priority);
        }

        return 1;
    }
}

int Msg::send(kernel_pid_t target_pid)
{
    if (cpu_is_in_isr())
    {
        return send_in_isr(target_pid);
    }

    if (get<ThreadScheduler>().get_current_active_pid() == target_pid)
    {
        return send_to_self_queue();
    }

    return send(target_pid, 1 /* blocking */, cpu_irq_disable());
}

int Msg::try_send(kernel_pid_t target_pid)
{
    if (cpu_is_in_isr())
    {
        return send_in_isr(target_pid);
    }

    if (get<ThreadScheduler>().get_current_active_pid() == target_pid)
    {
        return send_to_self_queue();
    }

    return send(target_pid, 0 /* non-blocking */, cpu_irq_disable());
}

int Msg::send_to_self_queue(void)
{
    unsigned state = cpu_irq_disable();

    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    sender_pid = current_thread->get_pid();

    int result = current_thread->queued_msg(this);

    cpu_irq_restore(state);

    return result;
}

int Msg::send_in_isr(kernel_pid_t target_pid)
{
    if (!Thread::is_pid_valid(target_pid))
    {
        // TODO: warning pit is not valid
    }

    Thread *target_thread = get<ThreadScheduler>().get_thread_from_scheduler(target_pid);

    if (target_thread == NULL)
    {
        return -1;
    }

    sender_pid = KERNEL_PID_ISR;

    if (target_thread->get_status() == THREAD_STATUS_RECEIVE_BLOCKED)
    {
        Msg *target_msg = static_cast<Msg *>(target_thread->wait_data);

        *target_msg = *this;

        get<ThreadScheduler>().set_thread_status(target_thread, THREAD_STATUS_PENDING);

        get<ThreadScheduler>().enable_context_switch_request();

        return 1;
    }
    else
    {
        return target_thread->queued_msg(this);
    }
}

int Msg::send_receive(Msg *reply, kernel_pid_t target_pid)
{
    vcassert(get<ThreadScheduler>().get_current_active_pid() != target_pid);

    unsigned state = cpu_irq_disable();

    Thread *current_thread = get<ThreadScheduler>().get_current_active_thread();

    get<ThreadScheduler>().set_thread_status(current_thread, THREAD_STATUS_REPLY_BLOCKED);

    current_thread->wait_data = static_cast<void *>(reply);

    /* we re-use (abuse) reply for sending, because wait_data might be
     * overwritten if the target is not in RECEIVE_BLOCKED */

    *reply = *this;

    /* Send() blocks until reply received */
    return reply->send(target_pid, 1 /* blocking */, state);
}

int Msg::reply(Msg *reply)
{
    unsigned state = cpu_irq_disable();

    Thread *target_thread = get<ThreadScheduler>().get_thread_from_scheduler(sender_pid);

    vcassert(target_thread != NULL);

    if (target_thread->get_status() != THREAD_STATUS_REPLY_BLOCKED)
    {
        cpu_irq_restore(state);

        return -1;
    }

    Msg *target_msg = static_cast<Msg *>(target_thread->wait_data);

    *target_msg = *reply;

    get<ThreadScheduler>().set_thread_status(target_thread, THREAD_STATUS_PENDING);

    uint8_t target_prio = target_thread->get_priority();

    cpu_irq_restore(state);

    get<ThreadScheduler>().context_switch(target_prio);

    return 1;
}

int Msg::reply_in_isr(Msg *reply)
{
    Thread *target_thread = get<ThreadScheduler>().get_thread_from_scheduler(sender_pid);

    if (target_thread->get_status() != THREAD_STATUS_REPLY_BLOCKED)
    {
        return -1;
    }

    Msg *target_msg = static_cast<Msg *>(target_thread->wait_data);

    *target_msg = *reply;

    get<ThreadScheduler>().set_thread_status(target_thread, THREAD_STATUS_PENDING);

    get<ThreadScheduler>().enable_context_switch_request();

    return 1;
}

template <> inline Instance &Msg::get(void) const
{
    return get_instance();
}

template <typename Type> inline Type &Msg::get(void) const
{
    return get_instance().get<Type>();
}

} // namespace vc

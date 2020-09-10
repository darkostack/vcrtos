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

#include <assert.h>
#include <errno.h>

#include <vcrtos/cpu.h>
#include <vcrtos/instance.h>
#include <vcrtos/mutex.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>

typedef struct {
    mutex_t *mutex;
    thread_t *thread;
    int timeout;
} mutex_thread_t;

static void _callback_unlock_mutex(void *arg)
{
    mutex_t *mutex = (mutex_t *)arg;

    mutex_unlock(mutex);
}

void ztimer_sleep(ztimer_clock_t *clock, uint32_t duration)
{
    assert(!cpu_is_in_isr());

    mutex_t mutex;

    mutex_init(instance_get(), &mutex);

    ztimer_t timer = {
        .callback = _callback_unlock_mutex,
        .arg = (void *)&mutex,
    };

    ztimer_set(clock, &timer, duration);
    mutex_lock(&mutex);
}

void ztimer_periodic_wakeup(ztimer_clock_t *clock, ztimer_now_t *last_wakeup,
                            uint32_t period)
{
    unsigned state = cpu_irq_disable();
    ztimer_now_t now = ztimer_now(clock);
    ztimer_now_t target = *last_wakeup + period;
    ztimer_now_t offset = target - now;

    cpu_irq_restore(state);

    if (offset <= period) {
        ztimer_sleep(clock, offset);
        *last_wakeup = target;
    }
    else {
        *last_wakeup = now;
    }
}

static void _callback_wakeup(void *arg)
{
    thread_wakeup(instance_get(), (kernel_pid_t)((intptr_t)arg));
}

void ztimer_set_wakeup(ztimer_clock_t *clock, ztimer_t *timer, uint32_t offset, kernel_pid_t pid)
{
    ztimer_remove(clock, timer);

    timer->callback = _callback_wakeup;
    timer->arg = (void *)((intptr_t)pid);

    ztimer_set(clock, timer, offset);
}

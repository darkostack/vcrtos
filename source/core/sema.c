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

#include <errno.h>
#include <limits.h>

#include <vcrtos/sema.h>
#include <vcrtos/cpu.h>
#include <vcrtos/assert.h>
#include <vcrtos/ztimer.h>
#include <vcrtos/thread.h>
#include <vcrtos/mutex.h>

void sema_create(void *instance, sema_t *sema, unsigned int value)
{
    vcassert(sema != NULL);

    sema->value = value;
    sema->state = SEMA_OK;
    sema->instance = instance;

    mutex_init(instance, &sema->mutex);

    if (value == 0)
    {
        mutex_lock(&sema->mutex);
    }
}

void sema_destroy(sema_t *sema)
{
    vcassert(sema != NULL);

    sema->state = SEMA_DESTROY;
    mutex_unlock(&sema->mutex);
}

static int _sema_wait(sema_t *sema, int block, uint32_t us)
{
    vcassert(sema != NULL);

    if (sema->state != SEMA_OK)
    {
        return -ECANCELED;
    }

    int did_block = block;

    unsigned irqstate = cpu_irq_disable();

    while ((sema->value == 0) && block)
    {
        cpu_irq_restore(irqstate);

        if (us == 0)
        {
            mutex_lock(&sema->mutex);
        }
        else
        {
            uint32_t start = ztimer_now(ZTIMER_USEC);
            block = !mutex_lock_timeout(sema->instance, &sema->mutex, us);
            uint32_t elapsed = ztimer_now(ZTIMER_USEC) - start;

            if (elapsed < us)
            {
                us -= elapsed;
            }
            else
            {
                block = 0;
            }
        }

        if (sema->state != SEMA_OK)
        {
            mutex_unlock(&sema->mutex);
            return -ECANCELED;
        }

        irqstate = cpu_irq_disable();
    }

    if (sema->value == 0)
    {
        cpu_irq_restore(irqstate);
        return (did_block) ? -ETIMEDOUT : -EAGAIN;
    }

    unsigned int value = --sema->value;
    cpu_irq_restore(irqstate);

    /* only unlock mutex if it was a blocking operation */
    if (did_block && value > 0)
    {
        mutex_unlock(&sema->mutex);
    }

    return 0;
}

int sema_post(sema_t *sema)
{
    vcassert(sema != NULL);

    unsigned irqstate = cpu_irq_disable();

    if (sema->value == UINT_MAX)
    {
        cpu_irq_restore(irqstate);
        return -EOVERFLOW;
    }

    unsigned value = sema->value++;
    cpu_irq_restore(irqstate);

    if (value == 0)
    {
        mutex_unlock(&sema->mutex);
    }

    return 0;
}

int sema_wait_timed(sema_t *sema, uint64_t timeout)
{
    return _sema_wait(sema, (timeout != 0), timeout);
}

int sema_wait(sema_t *sema)
{
    return _sema_wait(sema, 1, 0);
}

int sema_try_wait(sema_t *sema)
{
    return _sema_wait(sema, 0, 0);
}

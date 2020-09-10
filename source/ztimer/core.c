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

#include <stdint.h>
#include <stdio.h>

#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>
#include <vcrtos/thread.h>
#include <vcrtos/ztimer.h>

static void _add_entry_to_list(ztimer_clock_t *clock, ztimer_base_t *entry);
static void _del_entry_from_list(ztimer_clock_t *clock, ztimer_base_t *entry);
static void _ztimer_update(ztimer_clock_t *clock);

#if VCRTOS_CONFIG_ZTIMER_EXTEND
static inline uint32_t _min_u32(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}
#endif

static unsigned _is_set(const ztimer_clock_t *clock, const ztimer_t *t)
{
    if (!clock->list.next)
    {
        return 0;
    }
    else
    {
        return (t->base.next || &t->base == clock->last);
    }
}

void ztimer_remove(ztimer_clock_t *clock, ztimer_t *timer)
{
    unsigned state = cpu_irq_disable();

    if (_is_set(clock, timer))
    {
        ztimer_update_head_offset(clock);
        _del_entry_from_list(clock, &timer->base);

        _ztimer_update(clock);
    }

    cpu_irq_restore(state);
}

void ztimer_set(ztimer_clock_t *clock, ztimer_t *timer, uint32_t val)
{
    unsigned state = cpu_irq_disable();

    ztimer_update_head_offset(clock);

    if (_is_set(clock, timer))
    {
        _del_entry_from_list(clock, &timer->base);
    }

    /* optionally subtract a configurable adjustment value */
    if (val > clock->adjust)
    {
        val -= clock->adjust;
    }
    else
    {
        val = 0;
    }

    timer->base.offset = val;

    _add_entry_to_list(clock, &timer->base);

    if (clock->list.next == &timer->base)
    {
#if VCRTOS_CONFIG_ZTIMER_EXTEND
        if (clock->max_value < UINT32_MAX)
        {
            val = _min_u32(val, clock->max_value >> 1);
        }
#endif
        clock->ops->set(clock, val);
    }

    cpu_irq_restore(state);
}

static void _add_entry_to_list(ztimer_clock_t *clock, ztimer_base_t *entry)
{
    uint32_t delta_sum = 0;

    ztimer_base_t *list = &clock->list;

    /* Jump past all entries which are set to an earlier target than the new entry */
    while (list->next)
    {
        ztimer_base_t *list_entry = list->next;
        if ((list_entry->offset + delta_sum) > entry->offset)
        {
            break;
        }
        delta_sum += list_entry->offset;
        list = list->next;
    }

    /* Insert into list */
    entry->next = list->next;
    entry->offset -= delta_sum;

    if (entry->next)
    {
        entry->next->offset -= entry->offset;
    }
    else
    {
        clock->last = entry;
    }

    list->next = entry;
}

#if VCRTOS_CONFIG_ZTIMER_EXTEND
static uint32_t _add_modulo(uint32_t a, uint32_t b, uint32_t mod)
{
    if (a < b)
    {
        a += mod + 1;
    }
    return a - b;
}

ztimer_now_t _ztimer_now_extend(ztimer_clock_t *clock)
{
    vcassert(clock->max_value);
    unsigned state = cpu_irq_disable();
    uint32_t lower_now = clock->ops->now(clock);
    clock->checkpoint += _add_modulo(lower_now, clock->lower_last, clock->max_value);
    clock->lower_last = lower_now;
    ztimer_now_t now = clock->checkpoint;
    cpu_irq_restore(state);
    return now;
}
#endif

void ztimer_update_head_offset(ztimer_clock_t *clock)
{
    uint32_t old_base = clock->list.offset;
    uint32_t now = ztimer_now(clock);
    uint32_t diff = now - old_base;

    ztimer_base_t *entry = clock->list.next;

    if (entry)
    {
        do {
            if (diff <= entry->offset)
            {
                entry->offset -= diff;
                break;
            }
            else
            {
                diff -= entry->offset;
                entry->offset = 0;
                if (diff)
                {
                    /* skip timers with offset==0 */
                    do {
                        entry = entry->next;
                    } while (entry && (entry->offset == 0));
                }
            }
        } while (diff && entry);
    }

    clock->list.offset = now;
}

static void _del_entry_from_list(ztimer_clock_t *clock, ztimer_base_t *entry)
{
    ztimer_base_t *list = &clock->list;

    vcassert(_is_set(clock, (ztimer_t *)entry));

    while (list->next)
    {
        ztimer_base_t *list_entry = list->next;

        if (list_entry == entry)
        {
            if (entry == clock->last)
            {
                /* if entry was the last timer, set the clocks last to the
                 * previous entry, or NULL if that was the list ptr */
                clock->last = (list == &clock->list) ? NULL : list;
            }

            list->next = entry->next;

            if (list->next)
            {
                list_entry = list->next;
                list_entry->offset += entry->offset;
            }

            /* reset the entry's next pointer so _is_set() considers it unset */
            entry->next = NULL;
            break;
        }
        list = list->next;
    }
}

static ztimer_t *_now_next(ztimer_clock_t *clock)
{
    ztimer_base_t *entry = clock->list.next;

    if (entry && (entry->offset == 0))
    {
        clock->list.next = entry->next;
        if (!entry->next)
        {
            /* The last timer just got removed from the clock's linked list */
            clock->last = NULL;
        }
        return (ztimer_t *)entry;
    }
    else
    {
        return NULL;
    }
}

static void _ztimer_update(ztimer_clock_t *clock)
{
#if VCRTOS_CONFIG_ZTIMER_EXTEND
    if (clock->max_value < UINT32_MAX)
    {
        if (clock->list.next)
        {
            clock->ops->set(clock, _min_u32(clock->list.next->offset, clock->max_value >> 1));
        }
        else
        {
            clock->ops->set(clock, clock->max_value >> 1);
        }
#else
    if (0) {
#endif
    }
    else
    {
        if (clock->list.next)
        {
            clock->ops->set(clock, clock->list.next->offset);
        }
        else
        {
            clock->ops->cancel(clock);
        }
    }
}

void ztimer_handler(ztimer_clock_t *clock)
{
#if VCRTOS_CONFIG_ZTIMER_EXTEND || VCRTOS_CONFIG_ZTIMER_NOW64
    if ((sizeof(ztimer_clock_t) == 8) || clock->max_value < UINT32_MAX)
    {
        /* calling now triggers checkpointing */
        uint32_t now = ztimer_now(clock);

        if (clock->list.next)
        {
            uint32_t target = clock->list.offset + clock->list.next->offset;
            int32_t diff = (int32_t)(target - now);
            if (diff > 0)
            {
                clock->ops->set(clock, _min_u32(diff, clock->max_value >> 1));
                return;
            }
        }
        else
        {
            clock->ops->set(clock, clock->max_value >> 1);
            return;
        }
    }
#endif

    clock->list.offset += clock->list.next->offset;
    clock->list.next->offset = 0;

    ztimer_t *entry = _now_next(clock);

    while (entry)
    {
        entry->callback(entry->arg);
        entry = _now_next(clock);
        if (!entry)
        {
            /* See if any more alarms expired during callback processing */
            /* This reduces the number of implicit calls to clock->ops->now() */
            ztimer_update_head_offset(clock);
            entry = _now_next(clock);
        }
    }

    _ztimer_update(clock);

    if (!cpu_is_in_isr())
    {
        thread_arch_yield_higher();
    }
}

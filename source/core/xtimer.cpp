#include <vcrtos/assert.h>

#include "core/instance.hpp"
#include "core/xtimer.hpp"
#include "core/mutex.hpp"

namespace vc {

void XTimer::set(uint32_t offset)
{
    if (!callback)
    {
        return;
    }

    get<XTimerScheduler>().remove(this);

    if (offset < get<XTimerScheduler>().get_timer_backoff())
    {
        get<XTimerScheduler>().spin(offset);
        shoot();
    }
    else
    {
        uint32_t set_target = get<XTimerScheduler>().now() + offset;
        get<XTimerScheduler>().set_absolute(this, set_target);
    }
}

void XTimer::set64(uint32_t offset, uint32_t long_offset)
{
    get<XTimerScheduler>().set64(this, offset, long_offset);
}

void XTimer::remove(void)
{
    unsigned state = cpu_irq_disable();
    
    if (is_set())
    {
        get<XTimerScheduler>().remove(this);
    }

    cpu_irq_restore(state);
}

extern "C" void lltimer_callback(void *arg, int channel)
{
    (void) channel;
    Instance &instance = *static_cast<Instance *>(arg);
    instance.get<XTimerScheduler>().callback();
}

extern "C" __attribute__((weak)) int lltimer_init(unsigned int, unsigned long, void(*)(void *, int), void *)
{
    return 0;
}

XTimerScheduler::XTimerScheduler(Instance &instances)
    : _in_handler(0)
    , _long_count(0)
#if VCRTOS_CONFIG_XTIMER_MASK
    , _high_count(0)
#endif
    , _timer_list_head(NULL)
    , _overflow_list_head(NULL)
    , _long_list_head(NULL)
{
#if !VCRTOS_CONFIG_PLATFORM_LINUX
    lltimer_init(VCRTOS_CONFIG_XTIMER_DEV,
                 XTIMER_HZ,
                 lltimer_callback,
                 static_cast<void *>(&instances));

    lltimer_set(0xffffffff);
#endif

    _instance = static_cast<void *>(&instances);
}

void XTimerScheduler::set_absolute(XTimer *timer, uint32_t target)
{
    uint32_t time_now = now();

    timer->next = NULL;

    uint32_t offset = (target - time_now);

    if (offset <= XTIMER_BACKOFF)
    {
        /* backoff */
        spin_until(target);
        timer->shoot();
        return;
    }

    unsigned state = cpu_irq_disable();

    if (timer->is_set())
    {
        remove(timer);
    }

    timer->target = target;
    timer->long_target = _long_count;

    target = target - XTIMER_OVERHEAD;

    if (target < time_now)
    {
        /* 32bit target overflow, target is in next 32bit period */
        timer->long_target++;
    }

    if ((timer->long_target > _long_count) || !this_high_period(target))
    {
        add_timer_to_long_list(&_long_list_head, timer);
    }
    else
    {
        if (lltimer_mask(time_now) >= target)
        {
            add_timer_to_list(&_overflow_list_head, timer);
        }
        else
        {
            add_timer_to_list(&_timer_list_head, timer);

            if (_timer_list_head == timer)
            {
                lltimer_set(target);
            }
        }
    }

    cpu_irq_restore(state);
}

void XTimerScheduler::set64(XTimer *timer, uint32_t offset, uint32_t long_offset)
{
    if (!long_offset)
    {
        timer->set(offset);
    }
    else
    {
        unsigned state = cpu_irq_disable();

        if (timer->is_set())
        {
            remove(timer);
        }

        now_internal(&timer->target, &timer->long_target);

        timer->target += offset;
        timer->long_target += long_offset;

        if (timer->target < offset)
        {
            timer->long_target++;
        }

        add_timer_to_long_list(&_long_list_head, timer);

        cpu_irq_restore(state);
    }
}

void XTimerScheduler::remove(XTimer *timer)
{
    if  (_timer_list_head == timer)
    {
        uint32_t next;

        _timer_list_head = static_cast<XTimer *>(timer->next);

        if (_timer_list_head)
        {
            next = _timer_list_head->target - XTIMER_OVERHEAD;
        }
        else
        {
            next = lltimer_mask(0xffffffff);
        }

        lltimer_set(next);
    }
    else
    {
        if (!remove_timer_from_list(&_timer_list_head, timer))
        {
            if (!remove_timer_from_list(&_overflow_list_head, timer))
            {
                remove_timer_from_list(&_long_list_head, timer);
            }
        }
    }
}

void XTimerScheduler::callback(void)
{
    uint32_t next_target;
    uint32_t reference;

    _in_handler = 1;

    if (!_timer_list_head)
    {
        next_period();

        reference = 0;

        /* make sure timer counter also arrived in the next timer period */
        while (lltimer_now() == lltimer_mask(0xffffffff));
    }
    else
    {
        reference = lltimer_now();
    }

overflow:
    /* check if timers are close to expiring */
    while (_timer_list_head &&
           (time_left(lltimer_mask(_timer_list_head->target), reference) < XTIMER_ISR_BACKOFF))
    {
        /* make sure we don't fire too early */
        while (time_left(lltimer_mask(_timer_list_head->target), reference));

        XTimer *timer = _timer_list_head;

        _timer_list_head = static_cast<XTimer *>(timer->next);

        timer->target = 0;
        timer->long_target = 0;

        timer->shoot();
    }

    uint32_t time_now = lltimer_now() + XTIMER_ISR_BACKOFF;

    if (time_now < reference)
    {
        /* timer is overflow while executing callback */

        next_period();

        while (reference < lltimer_now());

        reference = 0;

        goto overflow;
    }

    if (_timer_list_head)
    {
        /* schedule callback on next timer target time */
        next_target = _timer_list_head->target - XTIMER_OVERHEAD;

        /* make sure we are not setting a time in the past */
        if (next_target < (now() + XTIMER_ISR_BACKOFF))
        {
            goto overflow;
        }
    }
    else
    {
        /* there is no timer planned for this timer period, schedule callback on
         * next overflow */

        next_target = lltimer_mask(0xffffffff);

        time_now = lltimer_now();

        /* check for the overflow again */

        if (time_now < reference)
        {
            next_period();
            reference = 0;
            goto overflow;
        }
        else
        {
            /* check if the end of this period is very soon */
            if (lltimer_mask(time_now + XTIMER_ISR_BACKOFF) < time_now)
            {
                /* spin until next period, then advance */
                while (lltimer_now() >= time_now);
                next_period();
                reference = 0;
                goto overflow;
            }
        }
    }

    _in_handler = 0;

    lltimer_set(next_target);
}

uint32_t XTimerScheduler::now(void)
{
#if VCRTOS_CONFIG_XTIMER_MASK
    uint32_t latched_high_count;
    uint32_t time_now;
    do {
        latched_high_count = _high_count;
        time_now = lltimer_now();
    } while (_high_count != latched_high_count);
    return latched_high_count | time_now;
#else
    return lltimer_now();
#endif
}

uint64_t XTimerScheduler::now64(void)
{
    uint32_t short_term, long_term;

    now_internal(&short_term, &long_term);

    return ((uint64_t)long_term << 32) + short_term;
}

void XTimerScheduler::spin(uint32_t offset)
{
    uint32_t start = lltimer_now();
#if VCRTOS_CONFIG_XTIMER_MASK
    offset = lltimer_mask(offset);
    while (lltimer_mask(lltimer_now() - start) < offset);
#else
    while ((lltimer_now() - start) < offset);
#endif
}

void XTimerScheduler::now_internal(uint32_t *short_term, uint32_t *long_term)
{
    uint32_t before, after, long_value;

    do {
        before = now();
        long_value = _long_count;
        after = now();
    } while (before > after);

    *short_term = after;
    *long_term = long_value;
}

extern "C" void sleep64_unlock_mutex_handler(void *arg)
{
    Mutex *mutex = static_cast<Mutex *>(arg);
    mutex->unlock();
}

void XTimerScheduler::sleep64(uint32_t offset, uint32_t long_offset)
{
    if (cpu_is_in_isr())
    {
        vcassert(!long_offset);
        spin(offset);
        return;
    }

    Instance &instance = *static_cast<Instance *>(_instance);

    Mutex mutex = Mutex(instance);
    XTimer timer = XTimer(instance, sleep64_unlock_mutex_handler, static_cast<void *>(&mutex));

    mutex.lock();
    timer.set64(offset, long_offset);
    mutex.lock();
}

void XTimerScheduler::add_timer_to_list(XTimer **timer_list_head, XTimer *timer)
{
    while (*timer_list_head && (*timer_list_head)->target <= timer->target)
    {
        timer_list_head = reinterpret_cast<XTimer **>(&((*timer_list_head)->next));
    }

    timer->next = *timer_list_head;
    *timer_list_head = timer;
}

void XTimerScheduler::add_timer_to_long_list(XTimer **timer_list_head, XTimer *timer)
{
    while (*timer_list_head &&
           (((*timer_list_head)->long_target < timer->long_target) ||
            (((*timer_list_head)->long_target == timer->long_target) &&
             ((*timer_list_head)->target <= timer->target))))
    {
        timer_list_head = reinterpret_cast<XTimer **>(&((*timer_list_head)->next));
    }

    timer->next = *timer_list_head;
    *timer_list_head = timer;
}

int XTimerScheduler::remove_timer_from_list(XTimer **timer_list_head, XTimer *timer)
{
    while (*timer_list_head)
    {
        if (*timer_list_head == timer)
        {
            *timer_list_head = static_cast<XTimer *>(timer->next);
            return 1;
        }
        timer_list_head = reinterpret_cast<XTimer **>(&((*timer_list_head)->next));
    }
    return 0;
}

extern "C" __attribute__((weak)) uint32_t lltimer_read(unsigned int)
{
    return 0;
}

uint32_t XTimerScheduler::lltimer_now(void)
{
    return lltimer_read(VCRTOS_CONFIG_XTIMER_DEV);
}

uint32_t XTimerScheduler::lltimer_mask(uint32_t val)
{
    return val & ~XTIMER_MASK;
}

extern "C" __attribute__((weak)) int lltimer_set_absolute(unsigned int, unsigned, unsigned int)
{
    return 0;
}

void XTimerScheduler::lltimer_set(uint32_t target)
{
    if (_in_handler)
    {
        return;
    }
    lltimer_set_absolute(VCRTOS_CONFIG_XTIMER_DEV, VCRTOS_CONFIG_XTIMER_CHAN, lltimer_mask(target));
}

int XTimerScheduler::this_high_period(uint32_t target)
{
#if VCRTOS_CONFIG_XTIMER_MASK
    return (target & XTIMER_MASK) == _high_count;
#else
    (void) target;
    return 1;
#endif
}

void XTimerScheduler::spin_until(uint32_t target)
{
#if VCRTOS_CONFIG_XTIMER_MASK
    target = lltimer_mask(target);
#endif
    while (lltimer_now() > target);
    while (lltimer_now() < target);
}

void XTimerScheduler::select_long_timers(void)
{
    XTimer *select_list_start = _long_list_head;
    XTimer *select_list_last = NULL;

    while (_long_list_head)
    {
        if ((_long_list_head->long_target <= _long_count) && this_high_period(_long_list_head->target))
        {
            select_list_last = _long_list_head;
            _long_list_head = static_cast<XTimer *>(_long_list_head->next);
        }
        else
        {
            break;
        }
    }

    if (select_list_last)
    {
        select_list_last->next = NULL;
    }

    if (_timer_list_head)
    {
        if (select_list_last)
        {
            _timer_list_head = merge_lists(_timer_list_head, select_list_start);
        }
    }
    else
    {
        if (select_list_last)
        {
            _timer_list_head = select_list_start;
        }
    }
}

void XTimerScheduler::next_period(void)
{
#if VCRTOS_CONFIG_XTIMER_MASK
    _high_count += ~XTIMER_MASK + 1;

    if (_high_count == 0)
    {
        _long_count++;
    }
#else
    _long_count++;
#endif

    _timer_list_head = _overflow_list_head;
    _overflow_list_head = NULL;

    select_long_timers();
}

uint32_t XTimerScheduler::time_left(uint32_t target, uint32_t reference)
{
    uint32_t time_now = lltimer_now();

    if (time_now < reference)
    {
        return 0;
    }

    if (target > time_now)
    {
        return target - time_now;
    }
    else
    {
        return 0;
    }
}

XTimer *XTimerScheduler::compare(XTimer *timer_a, XTimer *timer_b)
{
    if (timer_a && timer_b)
    {
        return ((timer_a->target <= timer_b->target) ? timer_a : timer_b);
    }
    else
    {
        return (timer_a ? timer_a : timer_b);
    }
}

XTimer *XTimerScheduler::merge_lists(XTimer *timer_a_head, XTimer *timer_b_head)
{
    XTimer *result_head = compare(timer_a_head, timer_b_head);
    XTimer *pos = result_head;

    while (1)
    {
        timer_a_head = static_cast<XTimer *>(timer_a_head->next);
        timer_b_head = static_cast<XTimer *>(timer_b_head->next);

        if (!timer_a_head)
        {
            pos->next = timer_b_head;
            break;
        }

        if (!timer_b_head)
        {
            pos->next = timer_a_head;
            break;
        }

        pos->next = compare(timer_a_head, timer_b_head);
        pos = static_cast<XTimer *>(pos->next);
    }

    return result_head;
}

template <> inline Instance &XTimer::get(void) const
{
    return get_instance();
}

template <typename Type> inline Type &XTimer::get(void) const
{
    return get_instance().get<Type>();
}

} // namespace vc

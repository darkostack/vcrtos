#ifndef CORE_TIMER_HPP
#define CORE_TIMER_HPP

#include <vcrtos/config.h>
#include <vcrtos/timer.h>
#include <vcrtos/cpu.h>

namespace vc {

class Instance;

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t instance_raw[];
#endif

class Timer : public timer_t
{
public:
    explicit Timer(Instance &instances, timer_handler_func_t cb, void *args)
    {
        next = NULL;
        target = 0;
        long_target = 0;
        callback = cb;
        arg = args;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        instance = static_cast<void *>(&instances);
#else
        (void)instances;
#endif
    }

    void set(uint32_t offset);

    void set64(uint32_t offset, uint32_t long_offset);

    int is_set(void) { return (target || long_target); }

    void shoot(void) { callback(arg); }

    void remove(void);

private:
    template <typename Type> inline Type &get(void) const;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &get_instance(void) const { return *static_cast<Instance *>(instance); }
#else
    Instance &get_instance(void) const { return *reinterpret_cast<Instance *>(&instance_raw); }
#endif
};

class TimerScheduler
{
public:
    explicit TimerScheduler(Instance &instances);

    void set_absolute(Timer *timer, uint32_t target);

    void set64(Timer *timer, uint32_t offset, uint32_t long_offset);

    void remove(Timer *timer);

    void callback(void);

    uint32_t now(void);

    uint64_t now64(void);

    void spin(uint32_t offset);

    uint32_t get_timer_backoff(void) { return TIMER_BACKOFF; }

    void sleep64(uint32_t offset, uint32_t long_offset);

private:
    void now_internal(uint32_t *short_term, uint32_t *long_term);

    static void add_timer_to_list(Timer **list_head, Timer *timer);

    static void add_timer_to_long_list(Timer **list_head, Timer *timer);

    static int remove_timer_from_list(Timer **list_head, Timer *timer);

    static uint32_t lltimer_now(void);

    static uint32_t lltimer_mask(uint32_t val);

    void lltimer_set(uint32_t target);

    int this_high_period(uint32_t target);

    void spin_until(uint32_t target);

    void select_long_timers(void);

    void next_period(void);

    uint32_t time_left(uint32_t target, uint32_t reference);

    Timer *compare(Timer *timer_a, Timer *timer_b);

    Timer *merge_lists(Timer *timer_a_head, Timer *timer_b_head);

    enum
    {
        TIMER_BACKOFF = VCRTOS_CONFIG_TIMER_BACKOFF,
        TIMER_OVERHEAD = VCRTOS_CONFIG_TIMER_OVERHEAD,
        TIMER_ISR_BACKOFF = VCRTOS_CONFIG_TIMER_ISR_BACKOFF,
        TIMER_PERIODIC_SPIN = (TIMER_BACKOFF * 2),
        TIMER_PERIODIC_RELATIVE = VCRTOS_CONFIG_TIMER_PERIODIC_RELATIVE,
        TIMER_MASK = VCRTOS_CONFIG_TIMER_MASK,
        TIMER_HZ = VCRTOS_CONFIG_TIMER_HZ,
        TIMER_SHIFT = VCRTOS_CONFIG_TIMER_SHIFT,
    };

    int _in_handler;
    uint32_t _long_count;
#if VCRTOS_CONFIG_TIMER_MASK
    uint32_t _high_count;
#endif
    Timer *_timer_list_head;
    Timer *_overflow_list_head;
    Timer *_long_list_head;

    void *_instance;
};

} // namespace vc

#endif /* CORE_TIMER_HPP */

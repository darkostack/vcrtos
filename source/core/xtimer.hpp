#ifndef CORE_TIMER_HPP
#define CORE_TIMER_HPP

#include <vcrtos/config.h>
#include <vcrtos/xtimer.h>
#include <vcrtos/cpu.h>

namespace vc {

class Instance;

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t instance_raw[];
#endif

class XTimer : public xtimer_t
{
public:
    explicit XTimer(Instance &instances, xtimer_handler_func_t cb, void *args)
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

class XTimerScheduler
{
public:
    explicit XTimerScheduler(Instance &instances);

    void set_absolute(XTimer *timer, uint32_t target);

    void set64(XTimer *timer, uint32_t offset, uint32_t long_offset);

    void remove(XTimer *timer);

    void callback(void);

    uint32_t now(void);

    uint64_t now64(void);

    void spin(uint32_t offset);

    uint32_t get_timer_backoff(void) { return XTIMER_BACKOFF; }

    void sleep64(uint32_t offset, uint32_t long_offset);

private:
    void now_internal(uint32_t *short_term, uint32_t *long_term);

    static void add_timer_to_list(XTimer **list_head, XTimer *timer);

    static void add_timer_to_long_list(XTimer **list_head, XTimer *timer);

    static int remove_timer_from_list(XTimer **list_head, XTimer *timer);

    static uint32_t lltimer_now(void);

    static uint32_t lltimer_mask(uint32_t val);

    void lltimer_set(uint32_t target);

    int this_high_period(uint32_t target);

    void spin_until(uint32_t target);

    void select_long_timers(void);

    void next_period(void);

    uint32_t time_left(uint32_t target, uint32_t reference);

    XTimer *compare(XTimer *timer_a, XTimer *timer_b);

    XTimer *merge_lists(XTimer *timer_a_head, XTimer *timer_b_head);

    enum
    {
        XTIMER_BACKOFF = VCRTOS_CONFIG_XTIMER_BACKOFF,
        XTIMER_OVERHEAD = VCRTOS_CONFIG_XTIMER_OVERHEAD,
        XTIMER_ISR_BACKOFF = VCRTOS_CONFIG_XTIMER_ISR_BACKOFF,
        XTIMER_PERIODIC_SPIN = (XTIMER_BACKOFF * 2),
        XTIMER_PERIODIC_RELATIVE = VCRTOS_CONFIG_XTIMER_PERIODIC_RELATIVE,
        XTIMER_MASK = VCRTOS_CONFIG_XTIMER_MASK,
        XTIMER_HZ = VCRTOS_CONFIG_XTIMER_HZ,
        XTIMER_SHIFT = VCRTOS_CONFIG_XTIMER_SHIFT,
    };

    int _in_handler;
    uint32_t _long_count;
#if VCRTOS_CONFIG_XTIMER_MASK
    uint32_t _high_count;
#endif
    XTimer *_timer_list_head;
    XTimer *_overflow_list_head;
    XTimer *_long_list_head;

    void *_instance;
};

} // namespace vc

#endif /* CORE_TIMER_HPP */

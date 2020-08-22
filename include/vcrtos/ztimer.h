#ifndef ZTIMER_H
#define ZTIMER_H

#include <stdint.h>

#include <vcrtos/config.h>
#include <vcrtos/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ztimer_base ztimer_base_t;

typedef struct ztimer_clock ztimer_clock_t;

struct ztimer_base {
    ztimer_base_t *next;
    uint32_t offset;
};

#if VCRTOS_CONFIG_ZTIMER_NOW64
typedef uint64_t ztimer_now_t;
#else
typedef uint32_t ztimer_now_t;
#endif

typedef struct {
    ztimer_base_t base;
    void (*callback)(void *arg);
    void *arg;
} ztimer_t;

typedef struct {
    void (*set)(ztimer_clock_t *clock, uint32_t val);
    uint32_t (*now)(ztimer_clock_t *clock);
    void (*cancel)(ztimer_clock_t *clock);
} ztimer_ops_t;

struct ztimer_clock {
    ztimer_base_t list;
    const ztimer_ops_t *ops;
    ztimer_base_t *last;
    uint32_t adjust;
#if VCRTOS_CONFIG_ZTIMER_EXTEND || VCRTOS_CONFIG_ZTIMER_NOW64
    uint32_t max_value;
    uint32_t lower_last;
    ztimer_now_t checkpoint;
#endif
};

void ztimer_handler(ztimer_clock_t *clock);

void ztimer_set(ztimer_clock_t *clock, ztimer_t *timer, uint32_t val);

void ztimer_remove(ztimer_clock_t *clock, ztimer_t *timer);

ztimer_now_t _ztimer_now_extend(ztimer_clock_t *clock);

static inline ztimer_now_t ztimer_now(ztimer_clock_t *clock)
{
#if VCRTOS_CONFIG_ZTIMER_NOW64
    if (1) {
#elif VCRTOS_CONFIG_ZTIMER_EXTEND
    if (clock->max_value < UINT32_MAX) {
#else
    if (0) {
#endif
        return _ztimer_now_extend(clock);
    }
    else
    {
        return clock->ops->now(clock);
    }
}

void ztimer_periodic_wakeup(ztimer_clock_t *clock, uint32_t *last_wakeup, uint32_t period);

void ztimer_sleep(ztimer_clock_t *clock, uint32_t duration);

void ztimer_set_wakeup(ztimer_clock_t *clock, ztimer_t *timer, uint32_t offset, kernel_pid_t pid);

void ztimer_set_timeout_flag(ztimer_clock_t *clock, ztimer_t *timer, uint32_t timeout);

void ztimer_update_head_offset(ztimer_clock_t *clock);

void ztimer_init(void);

#if VCRTOS_CONFIG_ZTIMER_EXTEND
static inline void ztimer_init_extend(ztimer_clock_t *clock)
{
    if (clock->max_value < UINT32_MAX)
    {
        clock->ops->set(clock, clock->max_value >> 1);
    }
}
#endif

extern ztimer_clock_t *const ZTIMER_USEC;

extern ztimer_clock_t *const ZTIMER_MSEC;

extern ztimer_clock_t *const ZTIMER_USEC_BASE;

extern ztimer_clock_t *const ZTIMER_MSEC_BASE;

#ifdef __cplusplus
}
#endif

#endif /* ZTIMER_H */

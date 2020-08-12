#ifndef VCRTOS_TIMER_H
#define VCRTOS_TIMER_H

#include <stdlib.h>
#include <stdint.h>

#include <vcrtos/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*timer_handler_func_t)(void *arg);

typedef struct timer
{
    struct timer *next;
    uint32_t target;
    uint32_t long_target;
    timer_handler_func_t callback;
    void *arg;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} timer_t;

typedef struct
{
    uint64_t ticks64;
} timer_ticks64_t;

typedef struct
{
    uint32_t ticks32;
} timer_ticks32_t;

void timer_init(timer_t *timer, timer_handler_func_t callback, void *arg);

timer_ticks32_t timer_now(void);

timer_ticks64_t timer_now64(void);

uint32_t timer_now_usec(void);

uint64_t timer_now_usec64(void);

void timer_set(timer_t *timer, uint32_t offset);

void timer_set64(timer_t *timer, uint32_t offset, uint32_t long_offset);

void timer_sleep_usec(uint32_t usec);

void timer_sleep_usec64(uint64_t usec);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_TIMER_H */

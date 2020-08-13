#ifndef VCRTOS_XTIMER_H
#define VCRTOS_XTIMER_H

#include <stdlib.h>
#include <stdint.h>

#include <vcrtos/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*xtimer_handler_func_t)(void *arg);

typedef struct xtimer
{
    struct xtimer *next;
    uint32_t target;
    uint32_t long_target;
    xtimer_handler_func_t callback;
    void *arg;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} xtimer_t;

void xtimer_init(void *instance, xtimer_t *timer, xtimer_handler_func_t callback, void *arg);

uint32_t xtimer_now_usec(void *instance);

uint64_t xtimer_now_usec64(void *instance);

void xtimer_set(xtimer_t *timer, uint32_t offset);

void xtimer_set64(xtimer_t *timer, uint32_t offset, uint32_t long_offset);

void xtimer_sleep_usec(void *instance, uint32_t usec);

void xtimer_sleep_usec64(void *instance, uint64_t usec);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_XTIMER_H */

#ifndef ZTIMER_PERIPH_TIMER_H
#define ZTIMER_PERIPH_TIMER_H

#include <vcrtos/ztimer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ztimer_clock_t super;
    unsigned int dev;
    uint16_t min;
} ztimer_periph_timer_t;

void ztimer_periph_timer_init(ztimer_periph_timer_t *clock, unsigned int dev,
                              unsigned long freq, uint32_t max_val);

#ifdef __cplusplus
}
#endif

#endif /* ZTIMER_PERIPH_TIMER_H */

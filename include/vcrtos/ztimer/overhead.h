#ifndef ZTIMER_OVERHEAD_H
#define ZTIMER_OVERHEAD_H

#include <vcrtos/ztimer.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t ztimer_overhead(ztimer_clock_t *clock, uint32_t base);

#ifdef __cplusplus
}
#endif

#endif /* ZTIMER_OVERHEAD_H */

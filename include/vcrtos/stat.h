#ifndef VCRTOS_STAT_H
#define VCRTOS_STAT_H

#include <stdint.h>

#include <vcrtos/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scheduler_stat
{
    uint32_t last_start;
    unsigned int schedules;
    uint64_t runtime_ticks;
} scheduler_stat_t;

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_STAT_H */

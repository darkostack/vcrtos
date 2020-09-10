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
